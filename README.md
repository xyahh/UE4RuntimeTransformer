# UE4RuntimeTransformer
A Runtime Gizmo Transformer tool helps you translate/rotate/scale objects in runtime! Easily provide editing tools to your final product!

This is a UE4 Plugin made using C++ and Blueprints in Unreal Engine 4.24.

Plugin targeted for both :
- Users that want to customize absolutely everything, from how the Gizmo looks like, to how it behaves and how it interacts with different objects
- Users that just want to quickly implement a Gizmo System in their game without having to customize much!

# Version 1.0

Translation, Rotation, Scaling Available for Single & Multiple Objects.
Most functionality can be overriden (in both Blueprints & C++) for custom additional logic.

World Space & Local Space are both available for Translation and Rotation. Scaling is restricted to only work in Local Space (Scaling in World Space is a bit counter intuitive and difficult to manage)

UFocusable Interface for specific objects that require specific logic when Focused(Selected), Unfocused (Unselected), and when there is a Delta Transform pending.

Content Assets to work with:
- Post Process Material for Object Selection
- Gizmo Meshes to make your own personalized Gizmo
- Default Example Gizmo Materials
- Example Blueprints for each Gizmo and the TransformerActor (to see how one can easily add additional personalized features)

Current Known Issues:
- When Rotating and Scaling, the Gizmos shake just a little bit. This can go unnoticed but
still an issue that needs fixing.
- For now, the Binaries are only compiled for Windows only. Those that wish to distribute to Linux or Mac should have to compile the plugin in those machines as well.

# Next Steps
- Fix known issues 
- Add a custom Snapping Feature for all Transformations

# Additional Steps
- Make a video tutorial about the plugin
- Make a documentation tutorial about the plugin
