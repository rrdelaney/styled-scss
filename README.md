<h1 align="center">
  <img height="200" src="https://github.com/rrdelaney/styled-scss/blob/master/website/static/img/styled_logo.svg">
</h1>

<pre align="center">
  $ npm install --global styled-scss
</pre>

<h4 align="center">
  <i>styled-scss compiles Sass with dynamic variables to a styled-components runtime</i>
</h4>

<hr>

<h3 align="center">Take your Sass file</h3>

```scss
// page.scss

$bgColor: black;

@styled.button SpecialButton($primary: bool, $color: string) {
  display: inline-block;
  background: $bgColor;
  border-color: $color;

  @if $primary {
    background-color: $color;
    border-color: $bgColor;
  }
}
```

<h3 align="center">Run styled-scss on it</h3>

<pre align="center">
$ styled page.scss
</pre>


<h3 align="center">Get styled-components</h3>

```js
// pageStyles.js

import styled from "styled-components";
export const SpecialButton = styled.button`
  display: inline-block;
  background: black;
  border-color: ${props => props.color};
  ${({ color, primary }) =>
    primary &&
    css`
      background-color: ${color};
      border-color: black;
    `};
`;
```

## Docs

<p><details>
<summary><b>Type generation</b></summary>

styled-scss can also generate type interfaces to go along with the code! Right now it can
only do Reason, but TypeScript and Flow should be supported soon

```
$ styled page.scss --types reason
```
</details></p>

<p><details>
<summary><b>I don't like styled-components though!</b></summary>

styled-scss can also output to an Emotion runtime! Just pass the `--runtime` flag to the CLI

```
$ styled page.scss --runtime emotion
```
</details></p>
