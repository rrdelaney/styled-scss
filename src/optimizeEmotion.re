let makeBabelConfig = fileName =>
  Babel_core.Options.options(
    ~plugins=[|
      Babel_core.Plugin.withOptions("babel-plugin-emotion", {"hoist": true}),
    |],
    ~filename=fileName,
    ~ast=true,
    (),
  );

let optimize = (ast, fileName) => {
  let config = makeBabelConfig(fileName);
  let optimizedAst = Babel_core.transformFromAstSync(ast, "", config);
  optimizedAst##ast;
};
