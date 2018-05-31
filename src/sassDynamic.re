open Belt;

module T = Babel_types;

module Traverse = Babel_traverse;

/** Takes a prop name and converts it to a JS variable. */
let rawPropName = propName => Js.String.replace("$", "", propName);

/** Parses an if condition in Sass to a Babel AST. */
let parseCondition = conditionStr => {
  let parsed = Babel_parser.parse(conditionStr);

  Traverse.traverse(
    parsed,
    Traverse.traverser(~enter=path => {
      let node = Traverse.Path.node(path);
      if (T.isIdentifier(node)) {
        let identifier = T.asIdentifier(node);
        let identifierName = T.Identifier.name(identifier);
        let isSassVariable = Js.String.startsWith("$", identifierName);
        if (isSassVariable) {
          T.Identifier.nameSet(identifier, rawPropName(identifierName));
        };
      };
    }),
  );

  T.assertFile(parsed);
  let parsedFile = T.asFile(parsed);
  T.assertProgram(T.File.program(parsedFile));
  let parsedProgram = T.asProgram(T.File.program(parsedFile));
  let parsedExpression = Array.getExn(T.Program.body(parsedProgram), 0);
  T.assertExpressionStatement(parsedExpression);
  let expressionStatement = T.asExpressionStatement(parsedExpression);
  let conditionNode = T.ExpressionStatement.expression(expressionStatement);

  conditionNode;
};
