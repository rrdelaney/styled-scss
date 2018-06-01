import commonjs from 'rollup-plugin-commonjs'
import resolve from 'rollup-plugin-node-resolve'

const dependencies = Object.keys(require('../package.json').dependencies)
const external = [...dependencies, 'path', 'fs', 'process']

export default {
  output: {
    format: 'cjs',
    exports: 'named'
  },
  external,
  plugins: [commonjs(), resolve()]
}
