open Belt;

module T = Babel_types;

/** Lightweight representation of a template string to be built. */
type template = {
  literals: array(string),
  expressions: array(T.node),
};

/** Base template state. */
let empty = {literals: [||], expressions: [||]};

/** Determines if the next literal to be added should be collapsed into the previous one. */
let shouldCollapseLiteral = ({literals, expressions}) =>
  Array.length(literals) > Array.length(expressions)
  && Array.length(literals) > 0;

/** Appends a raw string to the last literal in a template. */
let collapseLastLiteral = (raw, literals) =>
  Belt.Array.mapWithIndex(literals, (index, literal) =>
    if (index == Array.length(literals) - 1) {
      literal ++ raw;
    } else {
      literal;
    }
  );

/** Adds a literal to a template string. */
let addLiteral = (raw, template) => {
  let nextLiterals =
    if (shouldCollapseLiteral(template)) {
      collapseLastLiteral(raw, template.literals);
    } else {
      Array.concat(template.literals, [|raw|]);
    };
  {...template, literals: nextLiterals};
};

/** Adds an expression to a template string. */
let addExpression = (node, template) => {
  let nextExpressions = Array.concat(template.expressions, [|node|]);
  {...template, expressions: nextExpressions};
};

/** Creates an array of `T.node`s from template literals. */
let templateToLiteralNodes = template => {
  let literals =
    Array.map(template.literals, raw =>
      T.templateElement(T.value(~raw), false)
    );

  if (Array.length(literals) == Array.length(template.expressions)) {
    Array.concat(literals, [|T.templateElement(T.value(~raw=""), true)|]);
  } else {
    literals;
  };
};

/** Builds a JS template string from a `template` object. */
let build = template =>
  T.templateLiteral(templateToLiteralNodes(template), template.expressions);
