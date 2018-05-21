let makeBabelConfig = fileName => {
  "plugins": [|
    Babel_core.Plugin.withOptions("babel-plugin-emotion", {"hoist": true}),
  |],
  "filename": fileName,
};

let optimize = (code, fileName) => Babel_core.transformSync(
                                      code,
                                      makeBabelConfig(fileName),
                                    )##code;
