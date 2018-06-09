open Belt;

module Debug = {
  /** If debug mode is enabled. */
  let enabled = ref(false);

  /** Last stage number that was printed. */
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

  /** Prints a stage number, stage name, and AST. */
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

[@bs.deriving jsConverter]
type runtime = [ | [@bs.as "styled"] `Styled | [@bs.as "emotion"] `Emotion];

[@bs.deriving jsConverter]
type types = [ | [@bs.as "reason"] `Reason];

type options = {
  fileName: string,
  fileSource: string,
  debug: bool,
  runtime,
  optimize: bool,
  types: option(types),
};

type output = {
  source: string,
  fileName: string,
  types: option(string),
  typesFileName: option(string),
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

  let styledMetadata = StyledMetadata.extract(ast);
  Debug.printStage("Metadata Extracted", `Css(ast));

  let sassSource = Postcss.stringify(ast);
  let sassCompilation =
    Sass.renderSync(
      Sass.options(
        ~data=sassSource,
        ~includePaths=[|Node.Path.dirname(options.fileName)|],
        (),
      ),
    );
  let cssSource = sassCompilation |. Sass.Result.css |. Sass.Buffer.toString();
  Debug.printStage("Compiled with Sass", `Str(cssSource));

  let cssAst = Postcss.parse(cssSource, Postcss.Options.make());
  let nestedAst = NestCss.nestComponentRules(cssAst);
  Debug.printStage("Insert statements", `PrettyCss(nestedAst));

  let jsAst =
    ComponentBuilder.buildComponents(
      options.runtime,
      nestedAst,
      styledMetadata,
    );
  Debug.printStage("Generate Emotion", `PrettyJs(jsAst));

  let optimizedProgram =
    if (options.optimize) {
      let opt = OptimizeEmotion.optimize(jsAst, options.fileName);
      Debug.printStage("Optimize output", `Js(opt));
      opt;
    } else {
      Debug.printStage("Optimize output", `Str(Chalk.dim("Skipped")));
      jsAst;
    };

  let generated = Babel_generator.generate(optimizedProgram)##code;

  let outputSource =
    if (options.optimize) {
      generated;
    } else {
      Prettier.formatJs(generated);
    };

  let outputName =
    Node.Path.join([|
      Node.Path.dirname(options.fileName),
      Node.Path.basename_ext(options.fileName, ".scss") ++ "Styles.js",
    |]);

  let outputTypes =
    switch (options.types) {
    | Some(`Reason) =>
      Some(
        TypeGenerator.generateReasonDefinition(outputName, styledMetadata),
      )
    | None => None
    };

  let outputTypesFileName =
    switch (options.types) {
    | Some(`Reason) => Some(Js.String.replace(".js", ".re", outputName))
    | None => None
    };

  Debug.printStage(
    "Generate types",
    `Str(Option.getWithDefault(outputTypes, Chalk.dim("Skipped"))),
  );

  {
    source: outputSource,
    fileName: outputName,
    types: outputTypes,
    typesFileName: outputTypesFileName,
  };
};
