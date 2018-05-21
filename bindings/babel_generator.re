type output = {. "code": string};

[@bs.module "@babel/generator"]
external generate : Babel_types.node => output = "default";
