let lastStage = ref(1);

let printStageName = name => {
  Js.log(
    Chalk.green("=== Stage " ++ string_of_int(lastStage^) ++ ": ")
    ++ Chalk.yellow(name),
  );
  lastStage := lastStage^ + 1;
};

let printAst = ast => ast |> Postcss.stringify |> Js.log;

let prettyPrintAst = ast =>
  ast |> Postcss.stringify |> Prettier.formatCss |> Js.log;

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

let ast =
  Postcss.parse(fileSource, Postcss.Options.make(~from_=fileName, ()));

printStageName("Parsed AST");

printAst(ast);

let metadata = EmotionMetadata.extract(ast);

printStageName("Metadata Extracted");

printAst(ast);

let sassSource = Postcss.stringify(ast);

let compilation = Sass.renderSync(Sass.options(~data=sassSource, ()));

let cssSource = compilation |. Sass.Result.css |. Sass.Buffer.toString();

printStageName("Compiled with Sass");

Js.log(cssSource);

let cssAst = Postcss.parse(cssSource, Postcss.Options.make());

let nestedAst = Renest.nestComponentRules(cssAst);

printStageName("Insert statements");

prettyPrintAst(nestedAst);

let jsAst = ComponentBuilder.buildComponents(nestedAst, metadata);

let jsProgram = Babel_generator.generate(jsAst)##code;

printStageName("Generate Emotion");

Js.log(jsProgram);

let optimizedProgram = OptimizeEmotion.optimize(jsProgram, fileName);

printStageName("Optimize output");

Js.log(optimizedProgram);
