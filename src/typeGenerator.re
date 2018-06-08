open Belt;

let generateReasonDefinition = (outputName, metadata: StyledMetadata.metadata) => {
  let components = Map.String.valuesToArray(metadata.components);

  let componentDeclarations =
    Array.map(components, component =>
      ReasonablyTyped.DotTyped.ReactComponent({
        name: ReasonablyTyped.DotTyped.Identifier(component.name),
        type_:
          ReasonablyTyped.DotTyped.Object({
            properties:
              Map.String.toArray(component.props)
              |. Array.map(((propName, _propType)) =>
                   ReasonablyTyped.DotTyped.{
                     name:
                       ReasonablyTyped.DotTyped.Identifier(
                         Js.String.replace("$", "", propName),
                       ),
                     type_: ReasonablyTyped.DotTyped.String,
                     optional: true,
                   }
                 ),
            typeParameters: [||],
            extends: None,
          }),
      })
    );

  let styledModule =
    ReasonablyTyped.DotTyped.ModuleDeclaration({
      name: ReasonablyTyped.DotTyped.Identifier("./" ++ outputName),
      declarations: componentDeclarations,
    });

  ReasonablyTyped.GenerateReason.compile(~skipFmt=false, styledModule);
};
