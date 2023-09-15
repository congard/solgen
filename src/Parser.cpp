#include "Parser.h"
#include "utils/StringUtils.h"
#include "utils/FileUtils.h"
#include "utils/matches.h"

#include <iostream>

namespace solgen {
Parser::Parser(const CmdOptions &options, const Conf &conf)
    : m_options(options), m_conf(conf), m_index(clang_createIndex(1, 1)) {}

Parser::~Parser() {
    for (auto unit : m_units)
        clang_disposeTranslationUnit(unit);
    clang_disposeIndex(m_index);
}

static void parseUsrString(const std::string &usrString, bool* isVolatile, bool* isConst, bool *isRestrict) {
    size_t bangLocation = usrString.find_last_of('#');

    if (bangLocation == std::string::npos || bangLocation == usrString.length() - 1) {
        *isVolatile = *isConst = *isRestrict = false;
        return;
    }

    bangLocation++;
    int x = usrString[bangLocation];

    *isConst = x & 0x1;
    *isVolatile = x & 0x4;
    *isRestrict = x & 0x2;
}

struct ClassInfo {
    Class *clazz;
    Parser *builder;
};

static void readArgs(Args &args, CXCursor c) {
    int argsNum = clang_Cursor_getNumArguments(c);

    for (int i = 0; i < argsNum; ++i) {
        CXCursor arg = clang_Cursor_getArgument(c, i);
        args.emplace_back(Arg {
            .name = cx2str(clang_getCursorSpelling(arg)),
            .type = clang_getCursorType(arg),
            .hasDefault = !clang_Cursor_isNull(clang_Cursor_getVarDeclInitializer(arg))
        });
    }
}

static Options readOptions(const std::string &sig, const Conf &conf, CXCursor cursor) {
    Options result;

    // merge values from config if exists
    if (auto options = conf.getOptions(sig); options)
        result.options.insert(options->options.begin(), options->options.end());
    
    auto data = cx2str(clang_Cursor_getBriefCommentText(cursor));
    auto pos = data.find("#solgen");

    if (pos == std::string::npos)
        return result;

    pos += 6; // strlen("solgen")

    while (true) {
        pos = data.find('#', pos);

        if (pos == std::string::npos)
            break;

        ++pos; // skip #

        // read key
        std::string key;

        while (pos < data.size() && data[pos] != ' ') {
            key += data[pos];
            ++pos;
        }

        if (Options::isSwitcher(key)) {
            result[key] = true;
        } else if (key == Options::End) {
            break;
        } else { // read value
            std::string value = data.substr(pos + 1, data.find('#', pos) - 1 - pos - 1);
            result.setVal(key, value);
        }
    }

    return result;
}

static std::string getCtorSignature(const Type &type, const Class &parent) {
    std::string sig = type.getCanonicalName();
    sig.erase(0, 5); // 'void '
    sig.insert(0, parent.getType().getCanonicalName() + "::" + parent.getName());
    return sig;
}

static std::string getFunctionSignature(const Function &fun, const Class &parent) {
    std::string sig = fun.getType().getCanonicalName();
    auto pos = sig.find('(');
    sig.insert(pos, parent.getType().getCanonicalName() + "::" + fun.getName());
    return sig;
}

static std::string getFieldSignature(const Field &field, const Class &parent) {
    return field.getType().getCanonicalName() + " " + parent.getType().getCanonicalName() + "::" + field.getName();
}

static Enum buildEnum(CXCursor c, const Conf &conf) {
    Enum e;
    e.setName(cx2str(clang_getCursorSpelling(c)));
    e.setType(clang_getCursorType(c));
    e.setOptions(readOptions(e.getType().getCanonicalName(), conf, c));

    clang_visitChildren(c, [](CXCursor c, CXCursor parent, CXClientData clientData) {
        if (clang_getCursorKind(c) != CXCursor_EnumConstantDecl)
            return CXChildVisit_Continue;

        Enum &e = *reinterpret_cast<Enum *>(clientData);
        e.addKey(cx2str(clang_getCursorSpelling(c)));

        return CXChildVisit_Recurse;
    }, &e);

    return e;
}

void Parser::visitClass(CXCursor c) {
    bool hasChildren = false;

    clang_visitChildren(c, [](CXCursor c, CXCursor parent, CXClientData clientData) {
        *static_cast<bool*>(clientData) = true;
        return CXChildVisit_Break;
    }, &hasChildren);

    if (!hasChildren)
        return; // incomplete type

    Class clazz;
    clazz.setType(clang_getCursorType(c));

    if (m_allClasses.find(clazz.getType()) != m_allClasses.end())
        return; // class has been already parsed
    
    clazz.setName(cx2str(clang_getCursorSpelling(c)));
    clazz.setAbstract(clang_CXXRecord_isAbstract(c));
    clazz.setOptions(readOptions(clazz.getType().getCanonicalName(), m_conf, c));
    
    CXFile file;
    clang_getExpansionLocation(clang_getCursorLocation(c), &file, nullptr, nullptr, nullptr);
    clazz.setFile(cx2str(clang_getFileName(file)));
    clazz.setAbsFile(cx2str(clang_File_tryGetRealPathName(file)));

    Class *clazz_ptr {nullptr};
    Type parentType = clang_getCursorType(clang_getCursorSemanticParent(c));

    if (!parentType.getName().empty()) {
        clazz_ptr = &m_allClasses[parentType]->addClass(clazz);
    } else {
        clazz_ptr = &(m_classes[clazz.getType()] = clazz);
    }

    m_allClasses[clazz.getType()] = clazz_ptr;

    ClassInfo classInfo {clazz_ptr, this};

    clang_visitChildren(c, classVisitor, &classInfo);
}

CXChildVisitResult Parser::classVisitor(CXCursor c, CXCursor parent, CXClientData clientData) {
    auto &classInfo = *reinterpret_cast<ClassInfo*>(clientData);
    auto cursorKind = clang_getCursorKind(c);
    auto cursorSpelling = cx2str(clang_getCursorSpelling(c));
    auto semanticParent = clang_getCursorSemanticParent(c);
    auto parentKind = clang_getCursorKind(semanticParent);
    auto access = clang_getCXXAccessSpecifier(c);

    // cout << cursorSpelling << " : " << cx2str(clang_getCursorKindSpelling(cursorKind)) << "\n";

    if (parentKind == CXCursor_ClassDecl && access != CX_CXXPublic && cursorKind != CXCursor_Constructor)
        return CXChildVisit_Continue;

    switch (cursorKind) {
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl: {
            classInfo.builder->visitClass(c);
            return CXChildVisit_Continue;
        }
        case CXCursor_CXXBaseSpecifier: {
            Type cursorType = clang_getCursorType(c);
            auto baseIt = classInfo.builder->m_allClasses.find(cursorType);

            if (baseIt != classInfo.builder->m_allClasses.end()) {
                classInfo.clazz->addBase(Class::Base{baseIt->second, static_cast<Access>(access)});
            } else {
                // TODO: template classes
            }

            break;
        }
        case CXCursor_Constructor: { // TODO: detect deleted
            if (clang_CXXConstructor_isMoveConstructor(c))
                return CXChildVisit_Continue;
            
            Class *clazz = classInfo.clazz;
            Class::Constructor ctor;
            ctor.access = static_cast<Access>(access);
            ctor.options = readOptions(getCtorSignature(clang_getCursorType(c), *clazz), classInfo.builder->m_conf, c);
            readArgs(ctor.args, c);
            clazz->addConstructor(ctor);

            return CXChildVisit_Continue;
        }
        case CXCursor_EnumDecl: {
            Class *clazz = classInfo.clazz;
            clazz->addEnum(buildEnum(c, classInfo.builder->m_conf));
            return CXChildVisit_Continue;
        }
        case CXCursor_FieldDecl:
        case CXCursor_VarDecl: {
            Class *clazz = classInfo.clazz;
            auto ctype = clang_getCursorType(c);
            auto &field = clazz->addField({ctype, cursorSpelling});
            field.setConst(clang_isConstQualifiedType(ctype));
            field.setOptions(readOptions(getFieldSignature(field, *clazz), classInfo.builder->m_conf, c));
            break;
        }
        case CXCursor_CXXMethod: {
            Class *clazz = classInfo.clazz;

            Function fun;
            fun.setType(clang_getCursorType(c));
            fun.setName(cursorSpelling);
            fun.setOptions(readOptions(getFunctionSignature(fun, *clazz), classInfo.builder->m_conf, c));

            readArgs(fun.args(), c);

            auto usrStr = cx2str(clang_getCursorUSR(c));

            if (usrStr.find_last_of('#') == usrStr.size() - 2 && *(usrStr.end() - 1) == 'S') {
                fun.setStatic(true);
            } else {
                bool isVolatile, isConst, isRestrict;
                parseUsrString(usrStr, &isVolatile, &isConst, &isRestrict);
                fun.setVolatile(isVolatile);
                fun.setConst(isConst);
                fun.setRestrict(isRestrict);
            }

            clang_visitChildren(c, [](CXCursor c, CXCursor parent, CXClientData clientData) {
                Function &fun = *reinterpret_cast<Function *>(clientData);

                switch (clang_getCursorKind(c)) {
                    case CXCursor_CXXOverrideAttr:
                        fun.setOverride(true);
                        return CXChildVisit_Break;
                    default:
                        return CXChildVisit_Continue;
                }
            }, &fun);

            clazz->addFunction(fun);

            return CXChildVisit_Continue;
        }
        default: return CXChildVisit_Continue;
    }

    return CXChildVisit_Recurse;
}

static void printDiagnostics(CXTranslationUnit translationUnit){
    unsigned int nbDiag = clang_getNumDiagnostics(translationUnit);

    bool foundError = false;

    for (unsigned int currentDiag = 0; currentDiag < nbDiag; ++currentDiag) {
        CXDiagnostic diagnostic = clang_getDiagnostic(translationUnit, currentDiag);
        std::string errorStr = cx2str(clang_formatDiagnostic(diagnostic, clang_defaultDiagnosticDisplayOptions()));

        if (errorStr.find("error:") != std::string::npos)
            foundError = true;

        std::cerr << errorStr << std::endl;
    }

    if (foundError) {
        std::cerr << "Please resolve these issues and try again\n";
        exit(-1);
    }
}

void Parser::init() {
    m_args.emplace_back("-x");
    m_args.emplace_back("c++");

    for (auto &param : m_params) {
        m_args.emplace_back(param.first.c_str());
        m_args.emplace_back(param.second.c_str());
    }

    m_args.emplace_back(nullptr);
}

void Parser::parse(const File &file) {
    if (!FileUtils::shouldBeRegenerated(file, m_options) && !m_options.regenerateDerived) {
        // print it anyway, since an external tool like
        // CMake most likely needs all of them
        if (m_options.printPaths) {
            std::string sourceFile = FileUtils::getOutputPath(m_options.outputDir, file);
            std::cout << sourceFile << "\n";
        }

        return;
    }

    CXTranslationUnit unit = clang_parseTranslationUnit(
            m_index,
            file.c_str(), m_args.data(), static_cast<int>(m_args.size()) - 1,
            nullptr, 0,
            CXTranslationUnit_SkipFunctionBodies);

    if (unit == nullptr) {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        exit(-1);
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visitor, this);

    printDiagnostics(unit);
    m_units.emplace_front(unit);
}

CXChildVisitResult Parser::visitor(CXCursor c, CXCursor parent, CXClientData clientData) {
    auto builder = reinterpret_cast<Parser*>(clientData);
    auto cursorKind = clang_getCursorKind(c);
    auto cursorSpelling = cx2str(clang_getCursorSpelling(c));

    if (parent.kind == CXCursor_TranslationUnit && cursorKind != CXCursor_Namespace)
        return CXChildVisit_Continue;

    switch (cursorKind) {
        case CXCursor_Namespace:
            return matches(builder->m_filter, cursorSpelling) ? CXChildVisit_Recurse : CXChildVisit_Continue;
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl:
            builder->visitClass(c);
            break;
        case CXCursor_EnumDecl: {
            if (builder->m_enums.find(clang_getCursorType(c)) != builder->m_enums.end())
                break; // enum has been already parsed

            Enum e = buildEnum(c, builder->m_conf);

            CXFile file;
            clang_getExpansionLocation(clang_getCursorLocation(c), &file, nullptr, nullptr, nullptr);
            e.setAbsFile(cx2str(clang_File_tryGetRealPathName(file)));

            builder->m_enums[e.getType()] = e;

            break;
        }
        default: break;
    }

    return CXChildVisit_Continue;
}
}
