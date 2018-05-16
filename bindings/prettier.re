[@bs.deriving abstract]
type options = {
  [@bs.optional]
  parser: string,
};

[@bs.module "prettier"] external format : (string, options) => string = "";

let formatCss = source => format(source, options(~parser="css", ()));
