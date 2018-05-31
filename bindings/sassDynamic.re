open Belt;

/** Takes a prop name and converts it to a JS variable. */
let rawPropName = propName => Js.String.replace("$", "", propName);

/** Parses an if condition in Sass to a Babel AST. */
let parseCondition = conditionStr => {
  let parsed = Babel_parser.parse(conditionStr);

  Babel_traverse.traverse(
    parsed,
    Babel_traverse.traverser(~enter=path => {
      let node = Babel_traverse.Path.node(path);
      if (Babel_types.isIdentifier(node)) {
        let identifier = Babel_types.asIdentifier(node);
        let identifierName = Babel_types.Identifier.name(identifier);
        let isSassVariable = Js.String.startsWith("$", identifierName);
        if (isSassVariable) {
          Babel_types.Identifier.nameSet(
            identifier,
            rawPropName(identifierName),
          );
        };
      };
    }),
  );

  Babel_types.assertFile(parsed);
  let parsedFile = Babel_types.asFile(parsed);
  Babel_types.assertProgram(Babel_types.File.program(parsedFile));
  let parsedProgram =
    Babel_types.asProgram(Babel_types.File.program(parsedFile));
  let parsedExpression =
    Array.getExn(Babel_types.Program.body(parsedProgram), 0);
  Babel_types.assertExpressionStatement(parsedExpression);
  let expressionStatement =
    Babel_types.asExpressionStatement(parsedExpression);
  let conditionNode =
    Babel_types.ExpressionStatement.expression(expressionStatement);

  conditionNode;
};
