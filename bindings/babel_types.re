[@bs.deriving abstract]
type value = {raw: string};

type node;

[@bs.module "@babel/types"] external identifier : string => node = "";

[@bs.module "@babel/types"]
external memberExpression : (node, node) => node = "";

[@bs.module "@babel/types"]
external taggedTemplateExpression : (node, node) => node = "";

[@bs.module "@babel/types"]
external templateLiteral : (array(node), array(node)) => node = "";

[@bs.module "@babel/types"] external templateElement : value => node = "";

[@bs.module "@babel/types"] external program : array(node) => node = "";

[@bs.module "@babel/types"] external stringLiteral : string => node = "";

[@bs.module "@babel/types"]
external variableDeclarator : (node, node) => node = "";

[@bs.module "@babel/types"]
external variableDeclaration : (string, array(node)) => node = "";

[@bs.module "@babel/types"]
external exportNamedDeclaration : (node, array(node)) => node = "";
