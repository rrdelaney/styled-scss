module Flags = {
  [@bs.deriving abstract]
  type t = {
    [@bs.optional]
    snakeCase: bool,
    [@bs.optional]
    reason: bool,
    [@bs.optional]
    debug: bool,
    args: array(string),
  };
  external decode : Js.Json.t => t = "%identity";
};

let program =
  Commander.program
  |. Commander.version("1.0.0")
  |. Commander.usage("[options] <file ...>")
  |. Commander.option("--snake-case", "")
  |. Commander.option("--reason", "")
  |. Commander.option("--debug", "")
  |. Commander.parse(Commander.Process.argv)
  |. Flags.decode;

let files = Flags.args(program);

let debug = Flags.debug(program) |. Belt.Option.getWithDefault(false);

if (Array.length(files) == 0) {
  Js.Console.error("Must provide more than one file.");
  Node.Process.exit(-1);
};

let fileName = files[0];

let fileSource = Node.Fs.readFileAsUtf8Sync(fileName);

Compiler.compile({fileName, fileSource, debug});
