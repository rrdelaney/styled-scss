/** Config for `babel-plugin-emotion`. */
let emotionConfig = {"hoist": true};

/** Plugin definition for applying babel-plugin-emotion. */
let emotionPlugin =
  Babel_core.Plugin.withOptions("babel-plugin-emotion", emotionConfig);

/** Creates a Babel config for transforming a JS ast with `babel-plugin-emotion.`. */
let makeBabelConfig = fileName =>
  Babel_core.Options.options(
    ~plugins=[|emotionPlugin|],
    ~filename=fileName,
    ~ast=true,
    (),
  );

/** Optimizes a JS AST using Babel and the Emotion plugin. */
let optimize = (ast, fileName) => {
  let config = makeBabelConfig(fileName);
  let optimizedAst = Babel_core.transformFromAstSync(ast, "", config);
  optimizedAst##ast;
};
