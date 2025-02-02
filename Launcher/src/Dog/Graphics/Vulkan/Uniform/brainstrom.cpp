/*****************************************************************//**
 * \file   brainstrom.cpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \brief  Here's the header you wanted, travis
 *  *********************************************************************/

 // Brainstromin
 /*
 cuz i can't think unless i write all this down :p


 Alr so we currently got

 gpu:

 struct AnimationData {
     mat4 finalBonesMatrices[NL_MAX_BONES];
 };

 layout(set = 2, binding = 2) buffer readonly BoneBuffer {
   AnimationData bones[NL_MAX_MODELS];
 } animationData;

 cpp side:

 loop over all model components
 - if model has animation
   - insert to set of unique animations

 loop over all models
 - skip if model has no animation
 - if model in set of unique animations (depends on entity model ids)
   - update that animation, and get the final bone matrices for that animation
 - add final bones to a vector of all bones

 send vector of all bones to gpu

 problem: a ton of wasted space

 new way:

 similar to mesh data
 two buffers:
 one buffer holding data about animation data
 one buffer with the animation data itself

 AnimationData {
   mat4 finalBonesMatrices[NL_MAX_BONES];
 }
 AnimationMetadata {
   uint offset;
   uint numBones;
 }
 layout(set = 2, binding = 2) buffer readonly BoneBuffer {
   AnimationData bones[NL_MAX_MODELS];
 } animationData;

 layout(set = 2, binding = 3) buffer readonly AnimationMetadataBuffer {
   AnimationMetadata data[NL_MAX_MODELS];
 } animationMetadata;

 each instance will need index into metadata buffer. Will still just call it animationIndex

 // ok ez.


 __________________________________________________________________________________________________________________________________

 Ok so we have a vector of entities - each with a model, transform, potential animation and text

 current approach:

 In Instance system:

 Sort entities into groups:
 - by mesh ID and texture ID

 at the end of sorting into groups:
 - store numInstances of each group
 convert all instances in all groups into a flat vector (same order as in the groups as if it was laid out flat)

 Send that flat instance array to the gpu

 Then in render system:
 - create groups which store instance count, first instance, index count, first index, vertex offset, wireframe
   - Basically mesh info, and the instance numbers to match that flat instance array
 - execute compute shader
   - loop from i = first instance to first instance + instance count
   - get instance from flat instance array[i]
     - Try culling, if culled skip, otherwise generate draw command for this instance

 - submit vkCmdDrawIndexedIndirectCount


 ---------------
 Don't need to sort by texture id - why was I doing that in the first place? it makes no sense
 Mesh ID: was important when we didnt have unified vertex and index buffers. Now we do, so there's no need to group by mesh id

 In other words, we don't need any grouping of any sort

 However data that's shared between instances depending on mesh id:
 - first index
 - index count
 - vertex offset

 so each instance should carry mesh identifier. That mesh data should be stored in a new buffer that's easily indexable.
 - use that buffer in compute shader when making the indirect draw command
 - how to get instance index in the compute shader so we can submit draw command?
   - well, instance index would just be the current instance that the compute shader is processing, simple enough
   - would have to change our workgroup style
     - not by groups anymore, but by instances

 what does instance system even do anymore? old purpose was to collect them into groups
 - oh it still creates instance data im silly
 - also now has to set mesh id. Doesn't need to make groups anymore - can directly emplace them into the vector
 - don't need any "group" thing anymore at all actually - time to delete it all

 */