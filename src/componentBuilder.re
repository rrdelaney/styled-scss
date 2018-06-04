open Belt;

module T = Babel_types;

exception MustBeRoot;

let ruleIsIfCondition = (metadata: StyledMetadata.metadata, selector) =>
  Map.String.has(metadata.ifConditions, selector);

let rec buildTemplateFromNode =
        (
          ~propsInContext=false,
          component: StyledMetadata.component,
          metadata: StyledMetadata.metadata,
          node: Postcss.node,
          template: TemplateStringBuilder.template,
        ) =>
  switch (node) {
  | Postcss.Rule({selector, nodes})
      when ruleIsIfCondition(metadata, selector) =>
    let condition =
      Map.String.getExn(metadata.ifConditions, selector).condition;
    let conditionNode = SassDynamic.parseCondition(condition);

    let destructuredParams =
      T.objectPattern(
        component.props
        |. Map.String.keysToArray
        |. Array.map(prop =>
             T.objectProperty(
               T.identifier(SassDynamic.rawPropName(prop)),
               T.identifier(SassDynamic.rawPropName(prop)),
               false,
               true,
             )
           ),
      );
    let nodesCssTemplate =
      T.taggedTemplateExpression(
        T.identifier("css"),
        nodesToTemplateLiteral(
          ~propsInContext=true,
          component,
          metadata,
          nodes,
        ),
      );
    TemplateStringBuilder.addExpression(
      T.arrowFunctionExpression(
        [|destructuredParams|],
        T.logicalExpression("&&", conditionNode, nodesCssTemplate),
      ),
      template,
    );
  | node =>
    let nodeCss = Postcss.stringify(node) ++ ";";
    let splitOnVars = Js.String.split("_local_", nodeCss);
    Array.reduce(splitOnVars, template, (template, nodeStr) =>
      if (Js.String.startsWith("var_", nodeStr) && propsInContext) {
        let localName = Js.String.replace("var_", "", nodeStr);
        TemplateStringBuilder.addExpression(
          T.identifier(localName),
          template,
        );
      } else if (Js.String.startsWith("var_", nodeStr)) {
        let localName = Js.String.replace("var_", "", nodeStr);
        TemplateStringBuilder.addExpression(
          T.arrowFunctionExpression(
            [|T.identifier("props")|],
            T.memberExpression(
              T.identifier("props"),
              T.identifier(localName),
            ),
          ),
          template,
        );
      } else {
        TemplateStringBuilder.addLiteral(nodeStr, template);
      }
    );
  }
and nodesToTemplateLiteral =
    (~propsInContext=false, component, metadata, nodes) => {
  let template =
    Array.reduce(nodes, TemplateStringBuilder.empty, (template, node) =>
      buildTemplateFromNode(
        ~propsInContext,
        component,
        metadata,
        node,
        template,
      )
    );
  TemplateStringBuilder.build(template);
};

/** Builds a new emotion component from an array of css rules. */
let buildComponent =
    (
      component: StyledMetadata.component,
      metadata: StyledMetadata.metadata,
      ast: array(Postcss.node),
    ) =>
  T.taggedTemplateExpression(
    T.memberExpression(
      T.identifier("styled"),
      T.identifier(component.element),
    ),
    nodesToTemplateLiteral(component, metadata, ast),
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
      T.templateLiteral(
        [|T.templateElement(T.value(~raw=css), false)|],
        [||],
      ),
    ),
  );

/** Creates an import statement for the runtime being used. */
let buildImportStatement = runtime =>
  switch (runtime) {
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
    (runtime, ast: Postcss.node, metadata: StyledMetadata.metadata) => {
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
  let importStatement = buildImportStatement(runtime);
  T.program(Array.concat([|importStatement|], componentNodes));
};
