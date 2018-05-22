module Plugin = {
  type t;
  external fromString : string => t = "%identity";
  external fromOptions : Js.t({..}) => t = "%identity";
  external fromArray : array(t) => t = "%identity";
  let withOptions = (name, options) =>
    fromArray([|fromString(name), fromOptions(options)|]);
};

module Options = {
  [@bs.deriving abstract]
  type options = {
    plugins: array(Plugin.t),
    filename: string,
    [@bs.optional]
    ast: bool,
    [@bs.optional]
    code: bool,
  };
};

[@bs.module "@babel/core"]
external transform :
  (string, Options.options, Babel_generator.output => unit) => unit =
  "";

[@bs.module "@babel/core"]
external transformSync : (string, Options.options) => Babel_generator.output =
  "";

[@bs.module "@babel/core"]
external transformFromAst :
  (
    Babel_types.node,
    string,
    Options.options,
    Babel_generator.output => unit
  ) =>
  unit =
  "";

[@bs.module "@babel/core"]
external transformFromAstSync :
  (Babel_types.node, string, Options.options) => Babel_generator.output =
  "";
