open Belt;

module T = Babel_types;

exception MustBeRoot;

/** Tools for creating a JS template string programatically. */
module TemplateStringBuilder = {
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
      Array.concat(
        literals,
        [|T.templateElement(T.value(~raw=""), true)|],
      );
    } else {
      literals;
    };
  };

  /** Builds a JS template string from a `template` object. */
  let build = template =>
    T.templateLiteral(
      templateToLiteralNodes(template),
      template.expressions,
    );
};

let ruleIsIfCondition = (metadata: StyledMetadata.metadata, selector) =>
  Map.String.has(metadata.ifConditions, selector);

let rawPropName = propName => Js.String.replace("$", "", propName);

let rec buildTemplateFromNode =
        (
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
        nodesToTemplateLiteral(component, metadata, nodes),
      );
    TemplateStringBuilder.addExpression(
      T.arrowFunctionExpression(
        [|destructuredParams|],
        T.logicalExpression("&&", conditionNode, nodesCssTemplate),
      ),
      template,
    );
  | node =>
    TemplateStringBuilder.addLiteral(
      Postcss.stringify(node) ++ ";",
      template,
    )
  }
and nodesToTemplateLiteral = (component, metadata, nodes) => {
  let template =
    Array.reduce(nodes, TemplateStringBuilder.empty, (template, node) =>
      buildTemplateFromNode(component, metadata, node, template)
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
    T.memberExpression(T.identifier("styled"), T.identifier("div")),
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
