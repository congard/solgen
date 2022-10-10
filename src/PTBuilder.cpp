#include "PTBuilder.h"
#include "StringUtils.h"
#include "FileUtils.h"

#include <iostream>

using namespace std;

namespace solgen {
PTBuilder::PTBuilder(const CmdOptions &options, const Conf &conf)
    : options(options), conf(conf), m_index(clang_createIndex(1, 1)) {}

PTBuilder::~PTBuilder() {
    for (auto unit : m_units)
        clang_disposeTranslationUnit(unit);
    clang_disposeIndex(m_index);
}

void parseUsrString(const std::string &usrString, bool* isVolatile, bool* isConst, bool *isRestrict) {
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

bool matches(const std::regex &filter, const std::string &name) {
    return std::sregex_iterator(name.begin(), name.end(), filter) != std::sregex_iterator();
}

struct ClassInfo {
    Class *cl;
    PTBuilder *builder;
};

void readArgs(Args &args, CXCursor c) {
    int argsNum = clang_Cursor_getNumArguments(c);

    for (int i = 0; i < argsNum; ++i) {
        CXCursor arg = clang_Cursor_getArgument(c, i);
        args.emplace_front(Arg {
            cx2str(clang_getCursorSpelling(arg)), // name
            clang_getCursorType(arg), // type
            !clang_Cursor_isNull(clang_Cursor_getVarDeclInitializer(arg)) // hasDefault
        });
    }
}

void readOptions(GenOptions &result, const std::string &sig, const Conf &conf, CXCursor cursor) {
    // merge values from config if exists
    if (auto options = conf.getOptions(sig); options)
        result.options.insert(options->options.begin(), options->options.end());
    
    auto data = cx2str(clang_Cursor_getBriefCommentText(cursor));
    auto pos = data.find("#solgen");

    if (pos == std::string::npos)
        return;

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

        if (GenOptions::isSwitcher(key)) {
            result[key] = true;
        } else if (key == GenOptions::End) {
            break;
        } else { // read value
            std::string value = data.substr(pos + 1, data.find('#', pos) - 1 - pos - 1);
            result.setVal(key, value);
        }
    }
}

std::string getCtorSignature(const Type &type, const Class &parent) {
    std::string sig = type.getCanonicalName();
    sig.erase(0, 5); // 'void '
    sig.insert(0, parent.type.getCanonicalName() + "::" + parent.name);
    return sig;
}

std::string getFunctionSignature(const Function &fun, const Class &parent) {
    std::string sig = fun.type.getCanonicalName();
    auto pos = sig.find('(');
    sig.insert(pos, parent.type.getCanonicalName() + "::" + fun.name);
    return sig;
}

std::string getFieldSignature(const Field &field, const Class &parent) {
    return field.type.getCanonicalName() + " " + parent.type.getCanonicalName() + "::" + field.name;
}

Enum buildEnum(CXCursor c, const Conf &conf) {
    Enum e;
    e.name = cx2str(clang_getCursorSpelling(c));
    e.type = clang_getCursorType(c);
    readOptions(e.options, e.type.getCanonicalName(), conf, c);

    clang_visitChildren(c, [](CXCursor c, CXCursor parent, CXClientData clientData) {
        if (clang_getCursorKind(c) != CXCursor_EnumConstantDecl)
            return CXChildVisit_Continue;

        Enum &e = *reinterpret_cast<Enum *>(clientData);
        e.keys.emplace_front(cx2str(clang_getCursorSpelling(c)));

        return CXChildVisit_Recurse;
    }, &e);

    return e;
}

CXChildVisitResult classVisitor(CXCursor c, CXCursor parent, CXClientData clientData);

void visitClass(CXCursor c, PTBuilder *builder) {
    bool hasChildren = false;

    clang_visitChildren(c, [](CXCursor c, CXCursor parent, CXClientData clientData) {
        *static_cast<bool*>(clientData) = true;
        return CXChildVisit_Break;
    }, &hasChildren);

    if (!hasChildren)
        return; // incomplete type

    Class cl;
    cl.type = clang_getCursorType(c);

    if (builder->allClasses.find(cl.type) != builder->allClasses.end())
        return; // class has been already parsed
    
    cl.name = cx2str(clang_getCursorSpelling(c));
    cl.isAbstract = clang_CXXRecord_isAbstract(c);

    readOptions(cl.options, cl.type.getCanonicalName(), builder->conf, c);
    
    CXFile file;
    clang_getExpansionLocation(clang_getCursorLocation(c), &file, nullptr, nullptr, nullptr);
    cl.file = cx2str(clang_getFileName(file));
    cl.absFile = cx2str(clang_File_tryGetRealPathName(file));

    Class *cl_ptr {nullptr};
    Type parentType = clang_getCursorType(clang_getCursorSemanticParent(c));

    if (!parentType.getName().empty()) {
        cl_ptr = &builder->allClasses[parentType]->classes.emplace_front(cl);
    } else {
        cl_ptr = &(builder->classes[cl.type] = cl);
    }

    builder->allClasses[cl.type] = cl_ptr;

    ClassInfo classInfo {cl_ptr, builder};

    clang_visitChildren(c, classVisitor, &classInfo);
}

CXChildVisitResult classVisitor(CXCursor c, CXCursor parent, CXClientData clientData) {
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
            visitClass(c, classInfo.builder);
            return CXChildVisit_Continue;
        }
        case CXCursor_CXXBaseSpecifier: {
            Type cursorType = clang_getCursorType(c);
            auto baseIt = classInfo.builder->allClasses.find(cursorType);

            if (baseIt != classInfo.builder->allClasses.end()) {
                classInfo.cl->bases.emplace_front(Class::Base{baseIt->second, static_cast<Access>(access)});
            } else {
                // TODO: template classes
            }

            break;
        }
        case CXCursor_Constructor: { // TODO: detect deleted
            if (clang_CXXConstructor_isMoveConstructor(c))
                return CXChildVisit_Continue;
            
            Class *cl = classInfo.cl;
            Class::Constructor ctor;
            ctor.access = static_cast<Access>(access);
            readOptions(ctor.options, getCtorSignature(clang_getCursorType(c), *cl), classInfo.builder->conf, c);
            readArgs(ctor.args, c);
            cl->ctors.emplace_front(ctor);

            return CXChildVisit_Continue;
        }
        case CXCursor_EnumDecl: {
            Class *cl = classInfo.cl;
            cl->enums.emplace_front(buildEnum(c, classInfo.builder->conf));
            return CXChildVisit_Continue;
        }
        case CXCursor_FieldDecl:
        case CXCursor_VarDecl: {
            Class *cl = classInfo.cl;
            auto ctype = clang_getCursorType(c);
            auto &field = cl->fields.emplace_front(Field {
                cursorSpelling, // name
                ctype // type
            });
            field.isConst = clang_isConstQualifiedType(ctype);
            readOptions(field.options, getFieldSignature(field, *cl), classInfo.builder->conf, c);
            break;
        }
        case CXCursor_CXXMethod: {
            Class *cl = classInfo.cl;

            Function fun;
            fun.type = clang_getCursorType(c);
            fun.name = cursorSpelling;

            readOptions(fun.options, getFunctionSignature(fun, *cl), classInfo.builder->conf, c);
            readArgs(fun.args, c);

            auto usrStr = cx2str(clang_getCursorUSR(c));

            if (usrStr.find_last_of('#') == usrStr.size() - 2 && *(usrStr.end() - 1) == 'S') {
                fun.isStatic = true;
            } else {
                parseUsrString(usrStr, &fun.isVolatile, &fun.isConst, &fun.isRestrict);
            }

            clang_visitChildren(c, [](CXCursor c, CXCursor parent, CXClientData clientData) {
                Function &fun = *reinterpret_cast<Function *>(clientData);

                switch (clang_getCursorKind(c)) {
                    case CXCursor_CXXOverrideAttr:
                        fun.isOverride = true;
                        return CXChildVisit_Break;
                    default:
                        return CXChildVisit_Continue;
                }
            }, &fun);

            if (auto it = cl->functions.find(cursorSpelling); it != cl->functions.end()) {
                it->second.emplace_front(fun);
            } else {
                cl->functions[cursorSpelling] = {fun};
            }

            return CXChildVisit_Continue;
        }
        default: return CXChildVisit_Continue;
    }

    return CXChildVisit_Recurse;
}

void printDiagnostics(CXTranslationUnit translationUnit){
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

void PTBuilder::init() {
    m_args.emplace_back("-x");
    m_args.emplace_back("c++");

    for (auto &param : params) {
        m_args.emplace_back(param.first.c_str());
        m_args.emplace_back(param.second.c_str());
    }

    m_args.emplace_back(nullptr);
}

void PTBuilder::parse(const File &file) {
    if (options.printPaths) {
        std::string sourceFile = getOutputPath(options.outputDir, file, "cpp");
        cout << sourceFile << "\n";
    }

    if (!shouldBeRegenerated(file, options) && !options.regenerateDerived)
        return;

    CXTranslationUnit unit = clang_parseTranslationUnit(
            m_index,
            file.c_str(), m_args.data(), static_cast<int>(m_args.size()) - 1,
            nullptr, 0,
            CXTranslationUnit_SkipFunctionBodies);

    if (unit == nullptr) {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        exit(-1);
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData clientData) {
        auto builder = reinterpret_cast<PTBuilder*>(clientData);
        auto cursorKind = clang_getCursorKind(c);
        auto cursorSpelling = cx2str(clang_getCursorSpelling(c));

        if (parent.kind == CXCursor_TranslationUnit && cursorKind != CXCursor_Namespace)
            return CXChildVisit_Continue;

        switch (cursorKind) {
            case CXCursor_Namespace:
                return matches(builder->filter, cursorSpelling) ? CXChildVisit_Recurse : CXChildVisit_Continue;
            case CXCursor_ClassDecl:
            case CXCursor_StructDecl:
                visitClass(c, builder);
                break;
            case CXCursor_EnumDecl: {
                if (builder->enums.find(clang_getCursorType(c)) != builder->enums.end())
                    break; // enum has been already parsed

                Enum e = buildEnum(c, builder->conf);

                CXFile file;
                clang_getExpansionLocation(clang_getCursorLocation(c), &file, nullptr, nullptr, nullptr);
                auto absFile = cx2str(clang_File_tryGetRealPathName(file));
                
                builder->enums[e.type] = {e, absFile};
                
                break;
            }
            default: break;
        }
        
        return CXChildVisit_Continue;
    }, this);

    printDiagnostics(unit);
    m_units.emplace_front(unit);
}
}
