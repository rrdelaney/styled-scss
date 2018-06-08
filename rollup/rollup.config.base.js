import commonjs from 'rollup-plugin-commonjs'
import resolve from 'rollup-plugin-node-resolve'

const dependencies = Object.keys(require('../package.json').dependencies)
const external = [...dependencies, 'reason', 'path', 'fs', 'process']

export default {
  output: {
    format: 'cjs',
    exports: 'named',
    interop: false,
  },
  external,
  plugins: [commonjs(), resolve()]
}
