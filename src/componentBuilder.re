open Belt;

module T = Babel_types;

exception MustBeRoot;

let buildStyledTemplate = css =>
  T.taggedTemplateExpression(
    T.memberExpression(T.identifier("styled"), T.identifier("div")),
    T.templateLiteral([|T.templateElement(T.value(~raw=css))|], [||]),
  );

let buildComponent = (ast: array(Postcss.node)) => {
  let rules = Array.map(ast, Postcss.stringify);
  let styledBlock = Js.Array.joinWith("\n", rules);
  buildStyledTemplate(styledBlock);
};

let buildGlobalCss = css =>
  T.taggedTemplateExpression(
    T.identifier("injectGlobal"),
    T.templateLiteral([|T.templateElement(T.value(~raw=css))|], [||]),
  );

let buildConstExport = (exportName, exportNode) =>
  T.exportNamedDeclaration(
    T.variableDeclaration(
      "const",
      [|T.variableDeclarator(T.identifier(exportName), exportNode)|],
    ),
    [||],
  );

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
