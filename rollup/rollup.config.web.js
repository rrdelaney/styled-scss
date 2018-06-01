import json from 'rollup-plugin-json'
import alias from 'rollup-plugin-alias'
import base from './rollup.config.base'

export default {
  ...base,
  input: 'src/api.bs.js',
  output: {
    ...base.output,
    file: 'web.js'
  },
  external: [],
  plugins: [
    alias({
      prettier: require.resolve('prettier/standalone'),
      '@babel/core': require.resolve('@babel/standalone')
    }),
    ...base.plugins,
    json()
  ]
}
