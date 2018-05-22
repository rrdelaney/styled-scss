module Debug = {
  let enabled = ref(false);
  let lastStage = ref(1);
  let printStageName = name => {
    Js.log(
      Chalk.green("=== Stage " ++ string_of_int(lastStage^) ++ ": ")
      ++ Chalk.yellow(name),
    );
    lastStage := lastStage^ + 1;
  };
  let printCssAst = ast => ast |> Postcss.stringify |> Js.log;
  let prettyPrintCssAst = ast =>
    ast |> Postcss.stringify |> Prettier.formatCss |> Js.log;
  let printJsAst = ast =>
    ast |> Babel_generator.generate |> (output => Js.log(output##code));
  let prettyPrintJsAst = ast =>
    ast
    |> Babel_generator.generate
    |> (output => Prettier.formatJs(output##code))
    |> Js.log;
  let printStage = (name, ast) =>
    if (enabled^) {
      printStageName(name);
      switch (ast) {
      | `Css(t) => printCssAst(t)
      | `PrettyCss(t) => prettyPrintCssAst(t)
      | `Js(t) => printJsAst(t)
      | `PrettyJs(t) => prettyPrintJsAst(t)
      | `Str(t) => Js.log(t)
      };
    };
};

type options = {
  fileName: string,
  fileSource: string,
  debug: bool,
};

let compile = options => {
  Debug.enabled := options.debug;
  Debug.printStage("File source", `Str(options.fileSource));
  let ast =
    Postcss.parse(
      options.fileSource,
      Postcss.Options.make(~from_=options.fileName, ()),
    );
  Debug.printStage("Parsed AST", `Css(ast));
  let metadata = EmotionMetadata.extract(ast);
  Debug.printStage("Metadata Extracted", `Css(ast));
  let sassSource = Postcss.stringify(ast);
  let compilation = Sass.renderSync(Sass.options(~data=sassSource, ()));
  let cssSource = compilation |. Sass.Result.css |. Sass.Buffer.toString();
  Debug.printStage("Compiled with Sass", `Str(cssSource));
  let cssAst = Postcss.parse(cssSource, Postcss.Options.make());
  let nestedAst = Renest.nestComponentRules(cssAst);
  Debug.printStage("Insert statements", `PrettyCss(nestedAst));
  let jsAst = ComponentBuilder.buildComponents(nestedAst, metadata);
  Debug.printStage("Generate Emotion", `PrettyJs(jsAst));
  let optimizedProgram = OptimizeEmotion.optimize(jsAst, options.fileName);
  Debug.printStage("Optimize output", `Js(optimizedProgram));
};
