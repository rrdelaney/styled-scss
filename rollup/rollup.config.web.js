import json from 'rollup-plugin-json'
import alias from 'rollup-plugin-alias'
import commonjs from 'rollup-plugin-commonjs'
import resolve from 'rollup-plugin-node-resolve'
import base from './rollup.config.base'

export default {
  ...base,
  input: 'src/api.bs.js',
  output: {
    ...base.output,
    file: 'web.js',
    format: 'iife',
    name: 'styled'
  },
  external: [],
  plugins: [
    alias({
      prettier: require.resolve('prettier/standalone'),
      '@babel/core': require.resolve('@babel/standalone')
    }),
    commonjs(),
    resolve({
      browser: true
    }),
    json()
  ]
}
