type output = {
  .
  "code": string,
  "ast": Babel_types.node,
};

[@bs.module "@babel/generator"]
external generate : Babel_types.node => output = "default";
