open Belt;

let compile = (fileName, fileSource, runtimeStr) => {
  let debug = false;
  let optimize = false;
  let runtime =
    Compiler.runtimeFromJs(runtimeStr) |. Option.getWithDefault(`Styled);
  Compiler.compile({fileName, fileSource, runtime, debug, optimize});
};
