type ast;

[@bs.deriving abstract]
type options = {
  [@bs.as "from"] [@bs.optional]
  from_: string,
  [@bs.as "to"] [@bs.optional]
  to_: string,
};

[@bs.module "postcss"] external parse : (string, options) => ast = "";
