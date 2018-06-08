open Belt;

let compile = (fileName, fileSource, runtimeStr, typesStr) => {
  let debug = false;
  let optimize = false;
  let runtime =
    Compiler.runtimeFromJs(runtimeStr) |. Option.getWithDefault(`Styled);
  let types = Compiler.typesFromJs(typesStr);
  Compiler.compile({fileName, fileSource, runtime, types, debug, optimize});
};
