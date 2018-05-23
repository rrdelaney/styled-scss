open Belt;

module T = Babel_types;

exception MustBeRoot;

/**
  Creates a new tagged template string. The tag is hardcoded to be
  `styled.div` for now, and the string is the raw css provided.
 */
let buildStyledTemplate = css =>
  T.taggedTemplateExpression(
    T.memberExpression(T.identifier("styled"), T.identifier("div")),
    T.templateLiteral([|T.templateElement(T.value(~raw=css))|], [||]),
  );

/** Builds a new emotion component from an array of css rules. */
let buildComponent = (ast: array(Postcss.node)) => {
  let rules = Array.map(ast, Postcss.stringify);
  let styledBlock = Js.Array.joinWith("\n", rules);
  buildStyledTemplate(styledBlock);
};

/** Creates `export const $exportName = $exportNode;` ast node. */
let buildConstExport = (exportName, exportNode) =>
  T.exportNamedDeclaration(
    T.variableDeclaration(
      "const",
      [|T.variableDeclarator(T.identifier(exportName), exportNode)|],
    ),
    [||],
  );

/**
  Creates an `injectGlobal` tagged template literal with the
  raw css provided.
 */
let buildGlobalCss = css =>
  T.taggedTemplateExpression(
    T.identifier("injectGlobal"),
    T.templateLiteral([|T.templateElement(T.value(~raw=css))|], [||]),
  );

/**
  Builds a JS ast exporting emotion components from a css ast and metadata
  collected before being compiled with Sass.
 */
let buildComponents = (ast: Postcss.node, metadata: EmotionMetadata.metadata) =>
  switch (ast) {
  | Postcss.Root({nodes}) =>
    nodes
    |. Array.map(node =>
         switch (node) {
         | Postcss.Rule({selector, nodes})
             when Map.String.has(metadata.components, selector) =>
           buildConstExport("COMPONENT", buildComponent(nodes))
         | _ => buildConstExport("GLOBAL", buildGlobalCss(""))
         }
       )
    |. T.program
  | _ => raise(MustBeRoot)
  };