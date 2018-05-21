[@bs.deriving abstract]
type options = {
  [@bs.optional]
  parser: string,
};

[@bs.module "prettier"] external format : (string, options) => string = "";

let formatCss = source => format(source, options(~parser="css", ()));

[@bs.module "prettier"] external formatJs : string => string = "format";

module Debug = {
  type output = {. "formatted": string};
  [@bs.module "prettier"] [@bs.scope "__debug"]
  external formatAST : Babel_types.node => output = "";
};
