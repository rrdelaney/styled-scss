[@bs.deriving abstract]
type value = {raw: string};

type node;

[@bs.module "babel-types"] external identifier : string => node = "";

[@bs.module "babel-types"]
external memberExpression : (node, node) => node = "";

[@bs.module "babel-types"]
external taggedTemplateExpression : (node, node) => node = "";

[@bs.module "babel-types"]
external templateLiteral : (array(node), array(node)) => node = "";

[@bs.module "babel-types"] external templateElement : value => node = "";
/* t.taggedTemplateExpression(t.memberExpression(t.identifier('styled'), t.identifier('div')), t.templateLiteral([t.templateElement({raw: ' o '}, true)], [])) */
