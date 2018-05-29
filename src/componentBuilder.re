open Belt;

module T = Babel_types;

exception MustBeRoot;

type templatePart =
  | StringLiteral(string)
  | Expression(T.node);

let templatePartsToLiteral = parts => {
  let empty = ([||], [||]);
  let (literals, expressions) =
    Array.reduce(parts, empty, ((literals, expressions), part) =>
      switch (part) {
      | StringLiteral(raw)
          when
            Array.length(literals) >= Array.length(expressions)
            && Array.length(literals) > 0 => (
          Belt.Array.mapWithIndex(literals, (index, literal) =>
            if (index == Array.length(literals) - 1) {
              literal ++ raw;
            } else {
              literal;
            }
          ),
          expressions,
        )
      | StringLiteral(raw) => (Array.concat(literals, [|raw|]), expressions)
      | Expression(node) => (literals, Array.concat(expressions, [|node|]))
      }
    );
  let literals =
    Array.map(literals, raw => T.templateElement(T.value(~raw)));
  T.templateLiteral(literals, expressions);
};

let ruleIsIfCondition = (metadata: StyledMetadata.metadata, selector) =>
  Map.String.has(metadata.ifConditions, selector);

let rawPropName = propName => Js.String.replace("$", "", propName);

let rec buildTemplateFromNode =
        (
          component: StyledMetadata.component,
          metadata: StyledMetadata.metadata,
          node: Postcss.node,
        ) =>
  switch (node) {
  | Postcss.Rule({selector, nodes})
      when ruleIsIfCondition(metadata, selector) =>
    let condition =
      Map.String.getExn(metadata.ifConditions, selector).condition;
    let conditionNode =
      Babel_parser.getExpression(Babel_parser.parse(condition));
    let destructuredParams =
      T.objectPattern(
        component.props
        |. Map.String.keysToArray
        |. Array.map(prop =>
             T.objectProperty(
               T.identifier(rawPropName(prop)),
               T.identifier(rawPropName(prop)),
             )
           ),
      );
    let nodesCssTemplate =
      T.taggedTemplateExpression(
        T.identifier("css"),
        templatePartsToLiteral(
          Array.map(nodes, node =>
            buildTemplateFromNode(component, metadata, node)
          ),
        ),
      );
    Expression(
      T.arrowFunctionExpression(
        [|destructuredParams|],
        T.logicalExpression("&&", conditionNode, nodesCssTemplate),
      ),
    );
  | node => StringLiteral(Postcss.stringify(node) ++ ";")
  };

/** Builds a new emotion component from an array of css rules. */
let buildComponent =
    (
      component: StyledMetadata.component,
      metadata: StyledMetadata.metadata,
      ast: array(Postcss.node),
    ) =>
  T.taggedTemplateExpression(
    T.memberExpression(T.identifier("styled"), T.identifier("div")),
    templatePartsToLiteral(
      Array.map(ast, node =>
        buildTemplateFromNode(component, metadata, node)
      ),
    ),
  );

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
      [|
        T.importDefaultSpecifier(T.identifier("styled")),
        T.importSpecifier(
          T.identifier("injectGlobal"),
          T.identifier("injectGlobal"),
        ),
      |],
      T.stringLiteral("react-emotion"),
    )
  | `Styled =>
    T.importDeclaration(
      [|
        T.importDefaultSpecifier(T.identifier("styled")),
        T.importSpecifier(
          T.identifier("injectGlobal"),
          T.identifier("injectGlobal"),
        ),
      |],
      T.stringLiteral("styled-components"),
    )
  };

/** Builds a JS ast exporting emotion components from a css ast and metadata
collected before being compiled with Sass. */
let buildComponents =
    (outputMode, ast: Postcss.node, metadata: StyledMetadata.metadata) => {
  let nodes =
    switch (ast) {
    | Postcss.Root({nodes}) => nodes
    | _ => raise(MustBeRoot)
    };
  let componentNodes =
    Array.map(nodes, node =>
      switch (node) {
      | Postcss.Rule({selector, nodes}) as cssNode =>
        let hasComponent = Map.String.get(metadata.components, selector);
        switch (hasComponent) {
        | Some(component) =>
          buildConstExport(
            component.name,
            buildComponent(component, metadata, nodes),
          )
        | None => buildGlobalCss(Postcss.stringify(cssNode))
        };
      | cssNode => buildGlobalCss(Postcss.stringify(cssNode))
      }
    );
  let importStatement = buildImportStatement(outputMode);
  T.program(Array.concat([|importStatement|], componentNodes));
};
