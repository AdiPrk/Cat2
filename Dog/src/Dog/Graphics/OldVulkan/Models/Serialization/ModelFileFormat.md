# NLM - Version 2.0 - Model File Binary Format

NLM, or NiteLiteModel, is a binary file encoded in **little-endian** format, designed for efficient loading and updating of 3D models with animation data. This document outlines the binary layout of the file, detailing each section and the data it contains.

---

## Table of Contents

- [NLM - Version 2.0 - Model File Binary Format](#nlm---version-20---model-file-binary-format)
  - [Table of Contents](#table-of-contents)
  - [Header Section](#header-section)
  - [Axis-Aligned Bounding Box (AABB)](#axis-aligned-bounding-box-aabb)
  - [Mesh Data Section](#mesh-data-section)
    - [Mesh Info Map Header](#mesh-info-map-header)
    - [Mesh Header](#mesh-header)
    - [Vertex Data](#vertex-data)
    - [Index Data](#index-data)
    - [Embedded Texture Data (Optional)](#embedded-texture-data-optional)
  - [Animation Data Section (If Present)](#animation-data-section-if-present)
    - [Animation Metadata](#animation-metadata)
    - [Bone Information Map](#bone-information-map)
      - [Bone Info Map Header](#bone-info-map-header)
      - [Bone Info Entries](#bone-info-entries)
    - [Bone Map (Animation Keyframes)](#bone-map-animation-keyframes)
      - [Bone Map Header](#bone-map-header)
      - [Bone Entries](#bone-entries)
    - [Animation Hierarchy (Animation Nodes)](#animation-hierarchy-animation-nodes)
      - [Animation Node Header](#animation-node-header)
      - [Animation Nodes](#animation-nodes)
  - [Summary](#summary)

---

## Header Section

**Total Size:** 12 bytes

| Field                          | Size (bytes) | Description                                      |
|--------------------------------|--------------|--------------------------------------------------|
| **Hash**                       | 4            | Hash generated from the original model file      |
| **Magic Number** (`MODL`)      | 4            | Identifies the file as a model file.             |
| **Version** (`2`)              | 4            | Format version number.                           |
| **Animation Data Present**     | 4            | `1` if animation data is present; `0` otherwise. |

---

## Axis-Aligned Bounding Box (AABB)

**Total Size:** 24 bytes

| Field                      | Size (bytes) | Description                                        |
|----------------------------|--------------|----------------------------------------------------|
| **AABB Min** (`vec3`)      | 12           | Minimum corner of the bounding box `(x, y, z)`.    |
| **AABB Max** (`vec3`)      | 12           | Maximum corner of the bounding box `(x, y, z)`.    |

---

## Mesh Data Section

### Mesh Info Map Header

**Total Size:** 4 bytes

| Field                      | Size (bytes) | Description             |
|----------------------------|--------------|-------------------------|
| **Mesh Count** (`uint32`)  | 4            | Total number of meshes. |

Each mesh consists of vertex data, index data, and optional embedded texture information. The layout for each mesh is as follows:

### Mesh Header

**Total Size:** 8 bytes

| Field               | Size (bytes) | Description                     |
|---------------------|--------------|---------------------------------|
| **Vertex Count**    | 4            | Number of vertices in the mesh. |
| **Index Count**     | 4            | Number of indices in the mesh.  |

### Vertex Data

Each vertex contains the following attributes and is repeated **Vertex Count** times.

**Total Size per Vertex:** 76 bytes

| Field                         | Size (bytes) | Description                                               |
|-------------------------------|--------------|-----------------------------------------------------------|
| **Position** (`vec3`)         | 12           | Vertex position `(x, y, z)`.                              |
| **Color** (`vec3`)            | 12           | Vertex color `(r, g, b)`.                                 |
| **Normal** (`vec3`)           | 12           | Vertex normal `(x, y, z)`.                                |
| **UV Coordinates** (`vec2`)   | 8            | Texture UV coordinates `(u, v)`.                          |
| **Bone IDs** (`ivec4`)        | 16           | IDs of up to 4 bones influencing the vertex.              |
| **Bone Weights** (`vec4`)     | 16           | Weights of the corresponding bones.                       |

### Index Data

Indices define the triangles in the mesh.

**Total Size:** `4 * Index Count` bytes

| Field                    | Size (bytes) | Description                              |
|--------------------------|--------------|------------------------------------------|
| **Indices** (`uint32[]`) | 4 bytes each | Array of indices forming the mesh faces. |

### Embedded Texture Data (Optional)

If the mesh includes embedded texture data, it's stored here. If no texture data is present, `Texture Size` is set to `0`.

| Field                       | Size (bytes)    | Description                            |
|-----------------------------|-----------------|----------------------------------------|
| **Texture Size** (`uint32`) | 4               | Size of the texture data in bytes.     |
| **Texture Data**            | `Texture Size`  | Binary texture data blob.              |

---

## Animation Data Section (If Present)

This section is included only if the **Animation Data Present** flag in the header is set to `1`.

### Animation Metadata

**Total Size:** 8 bytes

| Field                           | Size (bytes) | Description                               |
|---------------------------------|--------------|-------------------------------------------|
| **Animation Duration** (`float`)| 4            | Total duration of the animation in ticks. |
| **Ticks Per Second** (`int`)    | 4            | Number of ticks per second.               |

### Bone Information Map

This map links bone IDs to their corresponding offset matrices.

#### Bone Info Map Header

**Total Size:** 4 bytes

| Field                      | Size (bytes) | Description             |
|----------------------------|--------------|-------------------------|
| **Bone Count** (`uint32`)  | 4            | Total number of bones.  |

#### Bone Info Entries

**Total Size per Bone:** 72 bytes

For each bone:

| Field                      | Size (bytes) | Description                                                        |
|----------------------------|--------------|--------------------------------------------------------------------|
| **Bone ID** (`int`)        | 4            | Unique integer ID for the bone.                                    |
| **Offset Matrix** (`mat4`) | 64           | Bone offset matrix (16 floats).                                    |
| **Bone Info ID** (`int`)   | 4            | Internal ID assigned to the bone in the bone info map.             |

### Bone Map (Animation Keyframes)

Contains keyframe data for each bone.

#### Bone Map Header

**Total Size:** 4 bytes

| Field                      | Size (bytes) | Description                          |
|----------------------------|--------------|--------------------------------------|
| **Bone Count** (`uint32`)  | 4            | Number of bones with animation data. |

#### Bone Entries

For each bone:

| Field                                   | Size (bytes)      | Description                                            |
|-----------------------------------------|-------------------|--------------------------------------------------------|
| **Node ID** (`int`)                     | 4                 | ID of the associated animation node.                   |
| **Bone ID** (`int`)                     | 4                 | ID of the bone (matches Bone Info ID).                 |
| **Position Keyframe Count** (`uint32`)  | 4                 | Number of position keyframes.                          |
| **Rotation Keyframe Count** (`uint32`)  | 4                 | Number of rotation keyframes.                          |
| **Scaling Keyframe Count** (`uint32`)   | 4                 | Number of scaling keyframes.                           |
| **Position Keyframes**                  | 16 bytes each     | Time (`float`), Position (`vec3`).                     |
| **Rotation Keyframes**                  | 20 bytes each     | Time (`float`), Rotation (`quat`).                     |
| **Scaling Keyframes**                   | 16 bytes each     | Time (`float`), Scale (`vec3`).                        |

**Keyframe Structures:**

- **Position Keyframe (16 bytes):**
  - **Time** (`float`): 4 bytes
  - **Position** (`vec3`): 12 bytes

- **Rotation Keyframe (20 bytes):**
  - **Time** (`float`): 4 bytes
  - **Rotation** (`quat`): 16 bytes (quaternion with `w, x, y, z`)

- **Scaling Keyframe (16 bytes):**
  - **Time** (`float`): 4 bytes
  - **Scale** (`vec3`): 12 bytes

### Animation Hierarchy (Animation Nodes)

Defines the hierarchical structure of the animation nodes.

#### Animation Node Header

**Total Size:** 4 bytes

| Field                      | Size (bytes) | Description                      |
|----------------------------|--------------|----------------------------------|
| **Node Count** (`uint32`)  | 4            | Total number of animation nodes. |

#### Animation Nodes

**Total Size per Node:** 72 bytes + (4 * Child Count) bytes

For each node:

| Field                        | Size (bytes)               | Description                                            |
|------------------------------|----------------------------|--------------------------------------------------------|
| **Node ID** (`int`)          | 4                          | Unique ID of the node.                                 |
| **Transformation** (`mat4`)  | 64                         | Local transformation matrix (16 floats).               |
| **Child Count** (`uint32`)   | 4                          | Number of child nodes.                                 |
| **Child Node IDs** (`int[]`) | 4 bytes each               | Array of child node IDs.                               |

---

## Summary

This binary format is designed for rapid loading and efficient runtime performance. By structuring the data to closely match in-memory representations, the model and animation data can be loaded without any processing overhead.

- **Header Section:** 12 bytes, containing magic number, version, and animation presence flag.
- **AABB Section:** 24 bytes, defining the model's bounding box.
- **Mesh Data Section:**
  - Mesh Info Map Header: 4 bytes.
  - For each mesh:
    - Mesh Header: 8 bytes.
    - Vertex Data: 76 bytes per vertex.
    - Index Data: 4 bytes per index.
    - Embedded Texture Data (Optional): Variable size.
- **Animation Data Section (If Present):**
  - Animation Metadata: 8 bytes.
  - Bone Information Map:
    - Bone Info Map Header: 4 bytes.
    - Bone Info Entries: 72 bytes per bone.
  - Bone Map (Animation Keyframes):
    - Bone Map Header: 4 bytes.
    - Bone Entries: Variable size depending on keyframes.
  - Animation Hierarchy:
    - Animation Node Header: 4 bytes.
    - Animation Nodes: 72 bytes + (4 * Child Count) bytes per node.

---

