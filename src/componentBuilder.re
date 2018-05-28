open Belt;

module T = Babel_types;

exception MustBeRoot;

/** Creates a new tagged template string. The tag is hardcoded to be
`styled.div` for now, and the string is the raw css provided. */
let buildStyledTemplate = css =>
  T.taggedTemplateExpression(
    T.memberExpression(T.identifier("styled"), T.identifier("div")),
    T.templateLiteral([|T.templateElement(T.value(~raw=css))|], [||]),
  );

/** Builds a new emotion component from an array of css rules. */
let buildComponent = (ast: array(Postcss.node)) => {
  let rules = Array.map(ast, Postcss.stringify);
  let styledBlock = Js.Array.joinWith(";\n", rules);
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

/** Creates an `injectGlobal` tagged template literal with the
raw css provided. */
let buildGlobalCss = css =>
  T.expressionStatement(
    T.taggedTemplateExpression(
      T.identifier("injectGlobal"),
      T.templateLiteral([|T.templateElement(T.value(~raw=css))|], [||]),
    ),
  );

let buildImportStatement = outputMode =>
  switch (outputMode) {
  | `Emotion =>
    T.importDeclaration(
      [|T.importDefaultSpecifier(T.identifier("styled"))|],
      T.stringLiteral("react-emotion"),
    )
  | `Styled =>
    T.importDeclaration(
      [|T.importDefaultSpecifier(T.identifier("styled"))|],
      T.stringLiteral("styled-components"),
    )
  };

/** Builds a JS ast exporting emotion components from a css ast and metadata
collected before being compiled with Sass. */
let buildComponents =
    (outputMode, ast: Postcss.node, metadata: StyledMetadata.metadata) =>
  switch (ast) {
  | Postcss.Root({nodes}) =>
    let componentNodes =
      Array.map(nodes, node =>
        switch (node) {
        | Postcss.Rule({selector, nodes}) as cssNode =>
          let hasComponent = Map.String.get(metadata.components, selector);
          switch (hasComponent) {
          | Some(component) =>
            buildConstExport(component.name, buildComponent(nodes))
          | None => buildGlobalCss(Postcss.stringify(cssNode))
          };
        | cssNode => buildGlobalCss(Postcss.stringify(cssNode))
        }
      );
    let importStatement = buildImportStatement(outputMode);
    T.program(Array.concat([|importStatement|], componentNodes));
  | _ => raise(MustBeRoot)
  };
