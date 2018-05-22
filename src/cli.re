module Flags = {
  [@bs.deriving abstract]
  type t = {
    [@bs.optional]
    snakeCase: bool,
    [@bs.optional]
    reason: bool,
    [@bs.optional]
    debug: bool,
  };
};

external parseFlags : Meow.flags => Flags.t = "%identity";

let input =
  Meow.make({|
  styled-scss

    --snake-case
    --reason
    --debug
|});

let files = Meow.input(input);

let flags = Meow.flags(input) |> parseFlags;

let debug = Flags.debug(flags) |. Belt.Option.getWithDefault(false);

if (Array.length(files) == 0) {
  Js.Console.error("Must provide more than one file.");
  Node.Process.exit(-1);
};

let fileName = files[0];

let fileSource = Node.Fs.readFileAsUtf8Sync(fileName);

Compiler.compile({fileName, fileSource, debug});
