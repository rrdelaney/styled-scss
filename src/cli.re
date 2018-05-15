[@bs.deriving abstract]
type flags = {
  [@bs.optional]
  snakeCase: bool,
  [@bs.optional]
  reason: bool,
};

external parseFlags : Meow.flags => flags = "%identity";

let input = Meow.make({|
  Backsass

    --snake-case
    --reason
|});

let files = Meow.input(input);

let flags = Meow.flags(input) |> parseFlags;

if (Array.length(files) == 0) {
  Js.Console.error("Must provide more than one file.");
  Node.Process.exit(-1);
};

let fileName = files[0];

let fileSource = Node.Fs.readFileAsUtf8Sync(fileName);

let ast = Postcss.parse(fileSource, Postcss.options(~from_=fileName, ()));

Js.log(ast);
