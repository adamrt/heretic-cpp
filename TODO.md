# Todos

- [X] Remove `GLOBAL_SCALE`
- [X] Increase the dimensions of the ortho proj matrix
  - This will let us dump the `GLOBAL_SCALE`
- [ ] Move the rendering to a smaller window to allow for more gui
- [ ] Improve the windows that are visible
  - Camera
  - Rendering
  - Lighting
  - Map
    - GNS Records
  - Scene
    - Weather
    - Time
    - Arrangement
    - Text (separate window)
    - Events (separate window)
  - Model
    - Stats (min, max, etc)
    - Translation, Rotation, Scale
- [ ] Fix directional lighting
  - We are using the position instead of the direction 
  - Unless we are already handling it in the shader (I forget)
- [ ] Figure out camera movement

