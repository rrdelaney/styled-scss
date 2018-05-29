type output = {. "program": {. "body": array(Babel_types.node)}};

type expressionOutput = {
  .
  "program": {. "body": array({. "expression": Babel_types.node})},
};

external asExpression : output => expressionOutput = "%identity";

[@bs.module "@babel/parser"] external parse : string => output = "";

let getNode = (output: output) =>
  Belt.Array.getExn(output##program##body, 0);

let getExpression = (output: output) => Belt.Array.getExn(
                                           asExpression(output)##program##body,
                                           0,
                                         )##expression;
