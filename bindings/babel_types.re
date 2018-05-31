[@bs.deriving abstract]
type value = {raw: string};

type node;

module Identifier = {
  [@bs.deriving abstract]
  type t = pri {mutable name: string};
};
external asIdentifier : node => Identifier.t = "%identity";
[@bs.module "@babel/types"] external identifier : string => node = "";
[@bs.module "@babel/types"] external isIdentifier : node => bool = "";
[@bs.module "@babel/types"] external assertIdentifier : node => unit = "";

[@bs.module "@babel/types"]
external memberExpression : (node, node) => node = "";

[@bs.module "@babel/types"]
external taggedTemplateExpression : (node, node) => node = "";

[@bs.module "@babel/types"]
external templateLiteral : (array(node), array(node)) => node = "";

[@bs.module "@babel/types"]
external templateElement : (value, bool) => node = "";

[@bs.module "@babel/types"] external stringLiteral : string => node = "";

[@bs.module "@babel/types"]
external variableDeclarator : (node, node) => node = "";

[@bs.module "@babel/types"]
external variableDeclaration : (string, array(node)) => node = "";

[@bs.module "@babel/types"]
external exportNamedDeclaration : (node, array(node)) => node = "";

[@bs.module "@babel/types"] external emptyStatement : unit => node = "";

[@bs.module "@babel/types"]
external importDeclaration : (array(node), node) => node = "";

[@bs.module "@babel/types"]
external importDefaultSpecifier : node => node = "";

[@bs.module "@babel/types"]
external importSpecifier : (node, node) => node = "";

[@bs.module "@babel/types"]
external arrowFunctionExpression : (array(node), node) => node = "";

[@bs.module "@babel/types"]
external logicalExpression : (string, node, node) => node = "";

[@bs.module "@babel/types"] external objectPattern : array(node) => node = "";

[@bs.module "@babel/types"]
external objectProperty : (node, node, bool, bool) => node = "";

module File = {
  [@bs.deriving abstract]
  type t = pri {program: node};
};
external asFile : node => File.t = "%identity";
[@bs.module "@babel/types"] external isFile : node => bool = "";
[@bs.module "@babel/types"] external assertFile : node => unit = "";

module Program = {
  [@bs.deriving abstract]
  type t = pri {body: array(node)};
};
external asProgram : node => Program.t = "%identity";
[@bs.module "@babel/types"] external program : array(node) => node = "";
[@bs.module "@babel/types"] external isProgram : node => bool = "";
[@bs.module "@babel/types"] external assertProgram : node => unit = "";

module ExpressionStatement = {
  [@bs.deriving abstract]
  type t = pri {expression: node};
};
external asExpressionStatement : node => ExpressionStatement.t = "%identity";
[@bs.module "@babel/types"] external expressionStatement : node => node = "";
[@bs.module "@babel/types"] external isExpressionStatement : node => bool = "";
[@bs.module "@babel/types"]
external assertExpressionStatement : node => unit = "";
