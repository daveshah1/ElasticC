#include "ECCParser.hpp"
#include "DataTypes.hpp"
#include "Evaluator.hpp"
#include "TemplateParser.hpp"
#include "Util.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stack>
#include <string>
using namespace std;

namespace ElasticC {
namespace Parser {
ECCParser::ECCParser(ParserState &_code, GlobalScope &_gs)
    : code(_code), gs(_gs) {
  // build the lists of tokens
  transform(unaryPrefixOperations.begin(), unaryPrefixOperations.end(),
            back_inserter(unaryPrefixOperTokens),
            [](const Operation &o) { return o.token; });
  transform(unaryPrefixOperations.begin(), unaryPrefixOperations.end(),
            back_inserter(unaryPrefixOperTypes),
            [](const Operation &o) { return o.type; });
  transform(binaryOperations.begin(), binaryOperations.end(),
            back_inserter(binaryAndPostfixOperTokens),
            [](const Operation &o) { return o.token; });
  transform(binaryOperations.begin(), binaryOperations.end(),
            back_inserter(binaryAndPostfixOperTypes),
            [](const Operation &o) { return o.type; });
  transform(unaryPostfixOperations.begin(), unaryPostfixOperations.end(),
            back_inserter(binaryAndPostfixOperTokens),
            [](const Operation &o) { return o.token; });
  transform(unaryPostfixOperations.begin(), unaryPostfixOperations.end(),
            back_inserter(binaryAndPostfixOperTypes),
            [](const Operation &o) { return o.type; });
  // initialise core types, etc
  IncludeFile("elasticcore.ech", true, true);
};

void ECCParser::ParseAll() {
  try {
    code.Skip();
    vector<Templates::TemplateParameter *> templateParams;
    while (!code.AtEnd()) {
      AttributeSet attr = ParseAttributes();
      if (code.PeekNext() == '#') {
        ParsePreprocessorDef();
        templateParams.clear();

      } else if (code.PeekNextIdentOrLiteral() == "const") {
        // Handle global constants

        // Parse one or more variable declarations
        // Don't pull const from the parser stream; leave
        // ParseVariableDeclaration to pick this up
        bool dummy_isRef;
        gs.statements.push_back(
            ParseVariableDeclaration(&gs, attr, dummy_isRef));
        templateParams.clear();
      } else if (code.PeekNextIdentOrLiteral() == "template") {
        templateParams = ParseTemplateDefinition(&gs);
      } else if (code.PeekNextIdentOrLiteral() == "struct") {
        ParseStructureDefinition(attr, templateParams);
        templateParams.clear();

      } else if (code.PeekNextIdentOrLiteral() == "block") {
        ParseHWBlock(attr);
        templateParams.clear();
      } else if (code.PeekNextIdentOrLiteral() == "typedef") {
        code.GetNextIdentOrLiteral(); // consume typedef keyword
        code.Skip();
        DataTypeSpecifier *baseType = ParseDataType(&gs);
        code.Skip();
        string typeName = code.GetNextIdentOrLiteral();
        if (typeName.size() == 0)
          throw parse_error("invalid name");
        typedefs[typeName] = baseType;
      } else if (IsDataTypeKeyword(code.PeekNextIdentOrLiteral())) {
        ParseFunction(attr, templateParams);
        templateParams.clear();
      } else if (code.PeekNext() == ';') {
        code.GetNext();
        templateParams.clear();
      } else {
        // try and print the most friendly error possible
        if (code.PeekNextIdentOrLiteral() != "") {
          PrintMessage(MSG_ERROR,
                       "unexpected token " + code.PeekNextIdentOrLiteral(),
                       code.GetLine());
        } else {
          PrintMessage(MSG_ERROR,
                       string("unexpected character ") + code.PeekNext(),
                       code.GetLine());
        }
        templateParams.clear();
      }
      code.Skip();
    }
  } catch (exception &e) {
    PrintMessage(MSG_ERROR, e.what(), code.GetLine());
  }
}

void ECCParser::ParsePreprocessorDef() {
  if (!code.CheckMatchAndGet('#')) {
    throw parse_error("invalid preprocessor definition");
  }
  string nextIdent = code.GetNextIdentOrLiteral();
  if (nextIdent == "include") {
    code.Skip();
    bool isSystemOnly;
    if (code.CheckMatchAndGet('<')) {
      isSystemOnly = true;
    } else if (code.CheckMatchAndGet('"')) {
      isSystemOnly = false;
    } else {
      throw parse_error("invalid ===#include=== statement");
    }
    string fileName;
    while (!((isSystemOnly && code.CheckMatchAndGet('>')) ||
             ((!isSystemOnly) && code.CheckMatchAndGet('"')))) {
      fileName += code.GetNext();
    }
    IncludeFile(fileName, isSystemOnly, false);
  } else if (nextIdent == "pragma") {
    code.Skip();
    string pragmaText;
    while ((!code.AtEnd()) && (!code.CheckMatchAndGet('\n'))) {
      pragmaText += code.GetNext();
    }
    pragmas.push_back(pragmaText);
  }
}

void ECCParser::IncludeFile(string fileName, bool systemOnly, bool quiet) {
  MessageLevel old_verbosity = verbosity;
  if (quiet) {
    verbosity = MSG_ERROR;
  }
  ParserState originalCode = code;
  string fullPath =
      FindFile(fileName, EnvironmentVars::ecc_incdir, !systemOnly);
  if (fullPath == "")
    throw parse_error("included file ===" + fileName + "=== not found");
  ifstream ins(fullPath);
  if (!ins.is_open()) {
    throw parse_error("failed to open included file ===" + fullPath + "===");
  } else {
    PrintMessage(MSG_NOTE, "parsing included file ===" + fullPath + "===",
                 code.GetLine());
  }
  ParserState hdCode(
      string((istreambuf_iterator<char>(ins)), istreambuf_iterator<char>()),
      fileName);
  code = hdCode;
  ParseAll();
  code = originalCode;
  verbosity = old_verbosity;
}

void ECCParser::ParseStructureDefinition(
    const AttributeSet &currentAttr,
    vector<Templates::TemplateParameter *> templ) {
  if (code.GetNextIdentOrLiteral() != "struct") {
    throw parse_error("bad structure definition");
  }
  code.Skip();
  UserStructure *strt = new UserStructure();
  strt->attributes = currentAttr;
  strt->params = templ;
  strt->parentContext = &gs;
  string name = code.GetNextIdentOrLiteral();
  if ((name.size() == 0) || (isdigit(name[0]))) {
    throw parse_error("invalid name for structure");
  }
  strt->name = name;
  code.Skip();
  if (!code.CheckMatchAndGet('{')) {
    throw parse_error("expected '{' after struct name");
  }
  code.Skip();
  while (!code.CheckMatchAndGet('}')) {
    code.Skip();
    bool dummy;
    VariableDeclaration *vardecl =
        ParseVariableDeclaration(strt, AttributeSet(), dummy);
    strt->members.insert(strt->members.end(),
                         vardecl->declaredVariables.begin(),
                         vardecl->declaredVariables.end());
    code.Skip();
  }

  gs.structures.push_back(strt);
};

void ECCParser::ParseFunction(const AttributeSet &currentAttr,
                              vector<Templates::TemplateParameter *> templ) {
  Function *func = new Function();
  func->parentContext = &gs;
  func->attributes = currentAttr;
  func->params = templ;
  code.Skip();
  if (code.PeekNextIdentOrLiteral() == "void") {
    code.GetNextIdentOrLiteral();
    func->is_void = true;
  } else {
    func->returnType = ParseDataType(func);
    func->is_void = false;
  }
  code.Skip();
  string functionName = code.GetNextIdentOrLiteral();
  if ((functionName.size() == 0) || (isdigit(functionName[0]))) {
    throw parse_error("invalid function name");
  }
  func->name = functionName;
  map<string, vector<Templates::TemplateParameter *>> dummy1;
  map<string, bool> dummy2;
  func->arguments = ParseArgumentList(func, dummy1, dummy2);
  code.Skip();
  func->body = ParseStatement(func);
  gs.functions.push_back(func);
}

AttributeSet ECCParser::ParseAttributes() {
  AttributeSet as;
  code.Skip();
  while ((!code.AtEnd()) && (code.PeekNext(2) == "[[")) {
    code.GetNext(2); // consume [[
    string attrStr = "";
    while (code.PeekNext(2) != "]]") {
      attrStr += code.GetNext();
    }
    as.AddAttributeFromStr(attrStr);
    code.GetNext(2); // consume ]]
    code.Skip();
  }
  return as;
}

Statement *ECCParser::ParseStatement(Context *ctx) {
  code.Skip();
  AttributeSet attr = ParseAttributes();
  Statement *result = NullStatement;
  int startLine = code.GetLine();
  if (code.PeekNext() == ';') {
    code.GetNext(); // consume ;
    result = NullStatement;
  } else if (code.PeekNext() == '{') {
    // it's a block
    code.GetNext(); // consume {
    Block *blk = ParseBlockContent(ctx);
    code.GetNext(); // consume }
    return blk;
  } else if (code.PeekNextIdentOrLiteral() == "if") {
    code.GetNextIdentOrLiteral(); // consume 'if'
    code.Skip();
    if (code.PeekNext() != '(') {
      throw parse_error(
          string("invalid if statement syntax (expecting '(', got ") +
          code.PeekNext() + string(")"));
    }
    code.GetNext(); // consume (

    Expression *cond = ParseExpression(vector<char>{')'}, ctx);
    code.GetNext(); // consume )
    code.Skip();
    Statement *clause_true = ParseStatement(ctx);
    code.Skip();
    Statement *clause_false = NullStatement;
    if (code.PeekNextIdentOrLiteral() == "else") {
      clause_false = ParseStatement(ctx);
    }
    result = new IfStatement(cond, clause_true, clause_false, attr);
  } else if (code.PeekNextIdentOrLiteral() == "for") {
    code.GetNextIdentOrLiteral(); // consume 'for'

    code.Skip();
    if (code.PeekNext() != '(') {
      throw parse_error(
          string("invalid for statement syntax (expecting '(', got ") +
          code.PeekNext() + string(")"));
    }
    code.GetNext();                               // consume (
    Statement *initialiser = ParseStatement(ctx); // this consumes the ;
    // create the for loop now so we can use it as a context later on
    ForLoop *forstmt = new ForLoop(initialiser, NullExpression, NullExpression,
                                   NullStatement, attr);
    forstmt->parentContext = ctx;

    forstmt->condition = ParseExpression(vector<char>{';'}, forstmt);
    code.GetNext(); // consume ;
    forstmt->incrementer = ParseExpression(vector<char>{')'}, forstmt);
    code.GetNext(); // consume )

    forstmt->body = ParseStatement(forstmt);

    result = forstmt;
  } else if (code.PeekNextIdentOrLiteral() == "while") {
    code.GetNextIdentOrLiteral(); // consume 'while'
    code.Skip();
    if (code.PeekNext() != '(') {
      throw parse_error(
          string("invalid while statement syntax (expecting '(', got ") +
          code.PeekNext() + string(")"));
    }
    code.GetNext(); // consume (

    Expression *cond = ParseExpression(vector<char>{')'}, ctx);
    code.GetNext(); // consume )
    code.Skip();
    Statement *body = ParseStatement(ctx);
    result = new WhileLoop(cond, body, attr);
  } else if (code.PeekNextIdentOrLiteral() == "return") {
    code.GetNextIdentOrLiteral(); // consume 'return'
    code.Skip();
    Expression *retval = NullExpression;
    if (code.PeekNext() != ';') {
      retval = ParseExpression(vector<char>{';'}, ctx);
    }
    code.GetNext(); // consume ;
    result = new ReturnStatement(retval);
  } else if (IsDataTypeKeyword(code.PeekNextIdentOrLiteral()) ||
             (code.PeekNextIdentOrLiteral() == "const") ||
             (code.PeekNextIdentOrLiteral() == "static")) {
    bool dummy_isRef;

    result = ParseVariableDeclaration(ctx, attr, dummy_isRef);
    // We have to consume the final ;
    if (code.GetNext() != ';') {
      throw parse_error("variable declaration must end in semicolon");
    }
  } else {
    // assume it's an expression
    Expression *expr = ParseExpression(vector<char>{';'}, ctx);
    code.GetNext(); // consume final ;
    result = expr;
  }
  result->codeLine = startLine;
  return result;
}

Block *ECCParser::ParseBlockContent(Context *ctx) {
  Block *block = new Block();
  block->parentContext = ctx;
  block->codeLine = code.GetLine();
  code.Skip();
  while (code.PeekNext() != '}') {
    block->content.push_back(ParseStatement(block));
    code.Skip();
  }
  return block;
}

bool ECCParser::IsDataTypeKeyword(string keyword) {
  vector<string> basicDataTypes = {"auto",   "unsigned", "signed", "ram", "rom",
                                   "stream", "stream2d", "fifo",   "void"};
  if (find(basicDataTypes.begin(), basicDataTypes.end(), keyword) !=
      basicDataTypes.end()) {
    return true;
  } else if (typedefs.find(keyword) != typedefs.end()) {
    return true;
  } else if (find_if(gs.structures.begin(), gs.structures.end(),
                     [&](UserStructure *s) { return s->name == keyword; }) !=
             gs.structures.end()) {
    return true;
  } else {
    return false;
  }
}

DataTypeSpecifier *ECCParser::ParseDataType(Context *ctx) {
  code.Skip();
  string typeName = code.GetNextIdentOrLiteral(true);
  DataTypeSpecifier *baseType;
  if (typeName == "") {
    throw parse_error(
        string("failed to parse data type: unexpected character ") +
        code.PeekNext());
  }
  // TODO: should types themselves specify and process their parameters?
  if (typeName == "auto") {
    baseType = new AutoTypeSpecifier();
  } else if (basicTypeNames.find(typeName) != basicTypeNames.end()) {
    baseType = new BasicDataTypeSpecifier(basicTypeNames.at(typeName));
    Templates::TemplateParser(baseType->Params()).Parse(this, ctx);
  } else if (typedefs.find(typeName) != typedefs.end()) {
    baseType = typedefs.at(typeName);
  } else if (find_if(gs.structures.begin(), gs.structures.end(),
                     [&](UserStructure *s) { return s->name == typeName; }) !=
             gs.structures.end()) {
    baseType = new StructureTypeSpecifier(
        *find_if(gs.structures.begin(), gs.structures.end(),
                 [&](UserStructure *s) { return s->name == typeName; }));
    Templates::TemplateParser(baseType->Params()).Parse(this, ctx);
  } else if (ctx->IsTemplateParameter(typeName)) {
    auto tpRef = ctx->FindTemplateParameter(typeName);
    baseType = new TemplateParamTypeSpecifier(tpRef.first, tpRef.second);
  } else {
    throw parse_error("failed to parse data type: no type named " + typeName);
  }
  // handle arrays - ElasticC allows arrays to be attached to the type as well
  // as the variable name
  return HandleArraySpecifier(baseType, ctx);
};

vector<Templates::TemplateParameter *>
ECCParser::ParseTemplateDefinition(Context *ctx) {
  code.Skip();
  vector<Templates::TemplateParameter *> params;
  if (code.PeekNextIdentOrLiteral() == "template") {
    code.GetNextIdentOrLiteral(); // consume "template" keyword
    code.Skip();
    if (!code.CheckMatchAndGet('<')) {
      throw parse_error("expected start of template parameter list");
    }
    code.Skip();
    while (!code.CheckMatchAndGet('>')) {
      if ((code.PeekNextIdentOrLiteral() == "class") ||
          (code.PeekNextIdentOrLiteral() == "typename")) {
        code.GetNextIdentOrLiteral();
        string name = code.GetNextIdentOrLiteral();
        if ((name.size() == 0) || (isdigit(name[0])))
          throw parse_error("invalid name for template parameter");
        params.push_back(new Templates::DataTypeParameter(name));
      } else {
        DataTypeSpecifier *paramType = ParseDataType(ctx);
        string name = code.GetNextIdentOrLiteral();
        if ((name.size() == 0) || (isdigit(name[0])))
          throw parse_error("invalid name for template parameter");
        params.push_back(new Templates::BitConstantParameter(name, paramType));
      }
      code.Skip();
      if (!code.CheckMatchAndGet(',')) {
        if (code.PeekNext() != '>') {
          throw parse_error("invalid template parameter list syntax");
        }
      }
      code.Skip();
    }
  }
  return params;
}

DataTypeSpecifier *ECCParser::HandleArraySpecifier(DataTypeSpecifier *baseType,
                                                   Context *ctx) {
  DataTypeSpecifier *withArray = baseType;
  code.Skip();
  vector<Expression *> arraySizes;
  while (code.CheckMatchAndGet('[')) {
    arraySizes.push_back(ParseExpression(vector<char>{']'}, ctx));
    code.GetNext(); // consume ]
    code.Skip();
  }
  // apply array sizes in reverse: this is actually correct
  for (int i = arraySizes.size() - 1; i >= 0; i--) {
    withArray = new ArrayTypeSpecifier(withArray, arraySizes[i]);
  }
  return withArray;
}

VariableDeclaration *ECCParser::ParseVariableDeclaration(
    Context *ctx, const AttributeSet &currentAttr, bool &isRef, bool oneOnly) {
  vector<Variable *> variables;
  vector<VariableQualifier> qualifiers;
  code.Skip();
  while (VariableQualifierStrings.find(code.PeekNextIdentOrLiteral()) !=
         VariableQualifierStrings.end()) {
    qualifiers.push_back(
        VariableQualifierStrings.at(code.GetNextIdentOrLiteral()));
    code.Skip();
  }

  DataTypeSpecifier *baseType = ParseDataType(ctx);
  code.Skip();
  if (code.CheckMatchAndGet('&'))
    isRef = true;
  do {
    code.Skip();
    string variableName = code.GetNextIdentOrLiteral();
    if (variableName == "") {
      throw parse_error(string("bad variable name - unexpected character ===") +
                        code.PeekNext() + string("==="));
    } else if (isdigit(variableName[0])) {
      throw parse_error(string("variable name cannot start with a number"));
    }
    if (ctx->DoesVariableExist(variableName)) {
      PrintMessage(
          MSG_WARNING,
          "redefining variable ===" + variableName + "===", code.GetLine());
    }
    code.Skip();
    // Handle arrays specified after the variable name
    DataTypeSpecifier *varType = HandleArraySpecifier(baseType, ctx);

    Variable *var = new Variable();
    var->name = variableName;
    var->qualifiers = qualifiers;
    var->type = varType;

    code.Skip();
    if (code.CheckMatchAndGet('=')) {
      code.Skip();
      var->initialiser = ParseExpression(vector<char>{';', ','}, ctx);
    } else {
      var->initialiser = NullExpression;
    }
    variables.push_back(var);
    code.Skip();
  } while ((!oneOnly) && (code.CheckMatchAndGet(',')));
  return new VariableDeclaration(variables, currentAttr);
}

vector<pair<Variable *, bool>> ECCParser::ParseArgumentList(
    Context *ctx,
    map<string, vector<Templates::TemplateParameter *>> &specialParams,
    map<string, bool> &specialParamFound) {
  vector<pair<Variable *, bool>> args;
  code.Skip();
  if (code.GetNext() != '(') {
    throw parse_error("invalid argument list, expected '('");
  }
  code.Skip();
  if (code.PeekNext() != ')') { // handle the empty list
    do {
      code.Skip();
      AttributeSet attr = ParseAttributes();
      code.Skip();
      string nextTok = code.PeekNextIdentOrLiteral();
      if (specialParams.find(nextTok) != specialParams.end()) {
        code.GetNextIdentOrLiteral();
        specialParamFound[nextTok] = true;
        Templates::TemplateParser(specialParams.at(nextTok)).Parse(this, ctx);
      } else {
        bool isByRef = false;
        VariableDeclaration *vardec =
            ParseVariableDeclaration(ctx, attr, isByRef, true);
        args.push_back(pair<Variable *, bool>(
            vardec->GetVariableDeclarations().at(0), isByRef));
      }
      code.Skip();

    } while (code.CheckMatchAndGet(',')); // handle multiple comma-seperated
                                          // args
  }
  code.Skip();

  if (code.GetNext() != ')') {
    throw parse_error("expected '')' at end of argument list");
  }

  return args;
}

ECCParser::OperationStackItem::OperationStackItem(OpStackItemType _type)
    : type(_type){};

ECCParser::OperationStackItem::OperationStackItem(OpStackItemType _type,
                                                  OperationType _oper)
    : type(_type), oper(_oper){};

void ECCParser::ApplyFromOpStack(stack<OperationStackItem> &opStack,
                                 stack<Expression *> &parseStack) {
  if (opStack.empty()) {
    throw parse_error("invalid expression (operation stack underflow)");
  }
  OperationStackItem top = opStack.top();
  opStack.pop();
  if (top.type == OpStackItemType::OPER) {
    const Operation *info = LookupOperation(top.oper);
    vector<Expression *> operands;
    for (int i = 0; i < info->num_params; i++) {
      if (parseStack.empty())
        throw parse_error("invalid expression (too few operands)");
      // Note reversed order here
      operands.insert(operands.begin(), parseStack.top());
      parseStack.pop();
    }
    parseStack.push(new BasicOperation(top.oper, operands));
  }
};

Expression *ECCParser::ParseExpression(vector<char> terminators, Context *ctx) {
  /*As before we use a modified shunting yard algorithm. It is modified to
   * improve function support, and support more operation types (unary minus,
   * etc)*/
  stack<OperationStackItem> opStack;
  stack<Expression *> parseStack;
  // Keep track of when the expression is finished
  bool isDone = false;
  // To support certain operation types keep track of whether the last parsed
  // token was an operation
  bool lastWasOperation = false;
  code.Skip();
  while (!isDone) {
    bool nextIsLiteral = false;
    if (isdigit(code.PeekNext())) {
      nextIsLiteral = true;
    } else if (lastWasOperation && (code.PeekNext() == '-') &&
               (isdigit(code.PeekNext(2)[1]))) {
      nextIsLiteral = true;
    }
    if (nextIsLiteral) {
      string literal = "";
      if (code.CheckMatchAndGet('-')) {
        literal += "-";
      }
      literal += code.GetNextIdentOrLiteral();
      parseStack.push(new Literal(BitConstant(literal)));
      lastWasOperation = false;
    } else if (code.CheckMatchAndGet('(')) {
      opStack.push(OperationStackItem(OpStackItemType::L_PAREN));
      lastWasOperation = true;
    } else if (code.PeekNext() == ')') {
      while ((!opStack.empty()) &&
             (opStack.top().type == OpStackItemType::L_PAREN)) {
        ApplyFromOpStack(opStack, parseStack);
      }
      if (opStack.empty()) {
        // if closing paren is an allowed terminator, then we are at the end of
        // the expression
        if (find(terminators.begin(), terminators.end(), ')') ==
            terminators.end()) {
          throw parse_error("mismatched parentheses");
        } else {
          isDone = true;
        }
      } else {
        opStack.pop(); // pop parenthesis from the operator stack
      }
      // definitely not the terminator so we can pull it from the parse stream
      // (if it is a terminator we need to keep it as other code might need to
      // see what the terminator was if more than one was specified)
      code.GetNext();
      lastWasOperation = false;
    } else if (code.PeekNextIdentOrLiteral().size() > 0) {
      string nextIdent = code.PeekNextIdentOrLiteral();
      // assume identifier as didn't look like literal earlier
      if (BuiltinTokens.find(nextIdent) != BuiltinTokens.end()) {
        // it's a builtin pseudo-function
        code.GetNextIdentOrLiteral(); // consume token
        code.Skip();
        if (!code.CheckMatchAndGet('(')) {
          throw parse_error("expected opening parentheses after builtin token");
        }
        Expression *operand = ParseExpression(vector<char>{')'}, ctx);
        if (!code.CheckMatchAndGet(')')) {
          throw parse_error("invalid builtin argument list");
        }
        parseStack.push(new Builtin(BuiltinTokens.at(nextIdent), operand));
      } else if (IsFunction(nextIdent)) {
        // it's a user-defined function
        code.GetNextIdentOrLiteral(); // consume function token
        Function *func =
            *find_if(gs.functions.begin(), gs.functions.end(),
                     [nextIdent](Function *f) { return f->name == nextIdent; });
        vector<Templates::TemplateParameter *> templParams =
            Templates::CloneParameterSet(func->params);
        Templates::TemplateParser(templParams).Parse(this, ctx);

        code.Skip();
        if (!code.CheckMatchAndGet('(')) {
          throw parse_error("expected argument list after function token");
        }
        vector<Expression *> args = ParseExpressionList(ctx, ')');
        code.Skip();
        if (!code.CheckMatchAndGet(')')) {
          throw parse_error("invalid function argument list");
        }
        FunctionCall *fcall = new FunctionCall(func, args);
        fcall->params = templParams;
        parseStack.push(fcall);
      } else {
        // assume it's a variable or variable-type construct
        parseStack.push(ParseVarExpression(ctx));
      }
      lastWasOperation = false;
    } else if (find(terminators.begin(), terminators.end(), code.PeekNext()) !=
               terminators.end()) {
      isDone = true;
    } else if (code.CheckMatchAndGet('{')) {
      // initialiser list
      vector<Expression *> items = ParseExpressionList(ctx, '}');
      if (!code.CheckMatchAndGet('}'))
        throw parse_error("invalid initialsier list");
      parseStack.push(new InitialiserList(items));
      lastWasOperation = false;
    } else {
      bool isOperation = false;
      OperationType operType;
      // check for operation here
      // what types of operations we look for depends on what the last token was
      if (lastWasOperation) {
        int index = code.FindToken(unaryPrefixOperTokens, true, false);
        if (index != -1) {
          isOperation = true;
          operType = unaryPrefixOperTypes[index];
        }
        lastWasOperation = true;
      } else {
        int index = code.FindToken(binaryAndPostfixOperTokens, true, false);
        if (index != -1) {
          isOperation = true;
          operType = binaryAndPostfixOperTypes[index];
          if (LookupOperation(operType)->num_params == 1) {
            lastWasOperation = false;
          } else {
            lastWasOperation = true;
          }
        }
      }
      if (isOperation) {
        // process operation
        const Operation *oper = LookupOperation(operType);
        while ((!opStack.empty()) &&
               (opStack.top().type == OpStackItemType::OPER)) {
          const Operation *top = LookupOperation(opStack.top().oper);
          if ((oper->right_associative) &&
              (oper->precedence > top->precedence)) {
            ApplyFromOpStack(opStack, parseStack);
          } else if ((!oper->right_associative) &&
                     (oper->precedence >= top->precedence)) {
            ApplyFromOpStack(opStack, parseStack);
          } else {
            break;
          }
        }
        opStack.push(OperationStackItem(OpStackItemType::OPER, operType));
      } else {
        throw parse_error(string("unexpected character ") + code.PeekNext() +
                          string(" in expression"));
      }
    }

    code.Skip();
  }

  while (opStack.size() > 0) {
    if (opStack.top().type != OpStackItemType::OPER)
      throw parse_error("mismatched parentheses");
    ApplyFromOpStack(opStack, parseStack);
  }

  if (parseStack.size() == 1) {
    return parseStack.top();
  } else if (parseStack.size() > 1) {
    throw parse_error("invalid expression (too many operands given?)");
  } else {
    return NullExpression;
  }
};

vector<Expression *> ECCParser::ParseExpressionList(Context *ctx, char term) {
  vector<Expression *> exprlist;
  if (code.PeekNext() == term)
    return exprlist;
  do {
    exprlist.push_back(ParseExpression(vector<char>{',', term}, ctx));
    code.Skip();
  } while (code.CheckMatchAndGet(','));
  return exprlist;
}

Expression *ECCParser::ParseVarExpression(Context *ctx) {
  string variableName = code.GetNextIdentOrLiteral();
  Expression *expr;
  if (ctx->IsTemplateParameter(variableName)) {
    auto tpRef = ctx->FindTemplateParameter(variableName);
    expr = new TemplateParamToken(tpRef.first, tpRef.second);
  } else {
    Variable *baseVariable = ctx->FindVariable(variableName);
    expr = new VariableToken(baseVariable);
  }
  code.Skip();
  while ((code.PeekNext() == '[') || (code.PeekNext() == '.')) {
    if (code.PeekNext() == '[') {
      vector<Expression *> index = ParseExpressionList(ctx, ']');
      if (!code.CheckMatchAndGet(']'))
        throw parse_error("expected end of array index");
      expr = new ArraySubscript(expr, index);
    } else if (code.PeekNext() == '.') {
      string memName = code.GetNextIdentOrLiteral();
      if (memName.size() == 0)
        throw parse_error("expected a structure member name");
      expr = new MemberAccess(expr, memName);
    }
    code.Skip();
  }
  return expr;
};

bool ECCParser::IsFunction(string token) {
  return any_of(gs.functions.begin(), gs.functions.end(),
                [token](Function *f) { return f->name == token; });
}

void ECCParser::ParseHWBlock(const AttributeSet &currentAttr) {
  HardwareBlock *hwBlock = new HardwareBlock();
  hwBlock->parentContext = &gs;
  hwBlock->attributes = currentAttr;
  code.GetNextIdentOrLiteral(); // Consume block keyword
  code.Skip();
  string blockName = code.GetNextIdentOrLiteral();
  if ((blockName.size() == 0) || (isdigit(blockName[0]))) {
    throw parse_error("invalid hardware block name");
  }
  PrintMessage(MSG_DEBUG, "parsing block " + blockName, code.GetLine());
  code.Skip();
  hwBlock->name = blockName;
  map<string, vector<Templates::TemplateParameter *>> specialInputs;
  map<string, bool> specialInputsFound;
  Templates::IntParameter clockFreq("frequency");
  specialInputs["clock"] = {&clockFreq};
  specialInputs["clken"] = {};
  specialInputs["input_valid"] = {};
  specialInputs["reset"] = {};
  vector<pair<Variable *, bool>> inputList =
      ParseArgumentList(hwBlock, specialInputs, specialInputsFound);
  transform(inputList.begin(), inputList.end(), back_inserter(hwBlock->inputs),
            [](pair<Variable *, bool> x) -> Variable * {
              if (x.second)
                throw parse_error("reference type not allowed as block input "
                                  "(consider using an output instead?)");
              else
                return x.first;
            });
  if (specialInputsFound["clock"])
    hwBlock->params.has_clock = true;
  SingleCycleEvaluator tempEval(&gs);
  if (clockFreq.wasSpecified)
    hwBlock->params.clock_freq = clockFreq.GetValue(&tempEval);
  if (specialInputsFound["clken"])
    hwBlock->params.has_cken = true;
  if (specialInputsFound["input_valid"])
    hwBlock->params.has_den = true;
  if (specialInputsFound["reset"])
    hwBlock->params.has_sync_rst = true;

  code.Skip();
  if (code.GetNext(2) != "=>")
    throw parse_error("missing => after block input list");
  code.Skip();

  map<string, vector<Templates::TemplateParameter *>> specialOutputs = {
      {"output_valid", {}}};
  map<string, bool> specialOutputsFound;

  vector<pair<Variable *, bool>> outputList =
      ParseArgumentList(hwBlock, specialOutputs, specialOutputsFound);
  transform(
      outputList.begin(), outputList.end(), back_inserter(hwBlock->outputs),
      [this](pair<Variable *, bool> x) -> Variable * {
        if (x.second)
          PrintMessage(MSG_WARNING, "ignoring reference type as block output",
                       code.GetLine());
        return x.first;
      });
  if (specialOutputsFound["output_valid"])
    hwBlock->params.has_den_out = true;

  hwBlock->body = ParseStatement(hwBlock);
  gs.blocks.push_back(hwBlock);
}
} // namespace Parser
} // namespace ElasticC
