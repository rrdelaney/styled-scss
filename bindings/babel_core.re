module Plugin = {
  type t;
  external fromString : string => t = "%identity";
  external fromOptions : Js.t({..}) => t = "%identity";
  external fromArray : array(t) => t = "%identity";
  let withOptions = (name, options) =>
    fromArray([|fromString(name), fromOptions(options)|]);
};

type options = {
  .
  "plugins": array(Plugin.t),
  "filename": string,
};

[@bs.module "@babel/core"]
external transform : (string, options, Babel_generator.output => unit) => unit =
  "";

[@bs.module "@babel/core"]
external transformSync : (string, options) => Babel_generator.output = "";
