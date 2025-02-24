# ForgECS
A simple ECS (Entity Component System) inspired by entt. This looks into and inherits the general concepts and ideas of an efficient ECS applying sparse sets, pagination, memory management and time consideration in the idea of creating a small headers only and sufficient API for game renderers.

The vault is the class that manages both the entities and the sparse sets. You would use vault to call functions that Forge(Create) entities and functions that apply and remove components to the entities.

SparseSet is a custom class that follows standard sparse set protocal that contains a dense and sparse array to map components to their id's.
