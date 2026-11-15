<!-- ue-mcp:begin (written by `npx ue-mcp init`; edit outside this block, it is replaced on every run) -->
## Unreal Engine through ue-mcp

This project's Unreal editor is driven through the `ue-mcp` MCP server.

- Call `project(get_status)` first (in the default context, `tools(action="call", category="project", method="get_status")`). It says whether the editor is connected and which project it has open.
- Find an action with `tools(action="search", query="...")`, read its parameters with `tools(action="describe", category="...", method="...")`, then call it. Prefer a dedicated action over `editor(execute_python)`.
- Mutating actions return a `rollback` record; read state back after a change instead of assuming it landed.

The server also serves these workflow guides as MCP prompts (prompts/get with the name) and resources (resources/read with the URI). Read the one that matches the task before starting it:

- `ue-mcp-animation` (`skill://ue-mcp/ue-mcp-animation`): Use when creating, modifying, retargeting, rigging, constraining, or validating skeletal animation through UE-MCP. Covers native UE 5.8+ IK Rig and Retargeter authoring, the Control Rig begin/read/apply/bake loop, generic contact locks, per-rig anatomical and mirrored-axis discovery, quaternion keying, deterministic bone analysis, and exact fixed-frame Unreal capture.
- `ue-mcp-blueprint` (`skill://ue-mcp/ue-mcp-blueprint`): Use when authoring or modifying Unreal Blueprint assets through ue-mcp. Covers the read-then-write discipline, node/pin wiring, component (SCS) hierarchy, variables, interfaces, compilation, and the difference between pin defaults and linked inputs. Pulls in any time the user asks to create, edit, or inspect a Blueprint.
- `ue-mcp-epic-routing` (`skill://ue-mcp/ue-mcp-epic-routing`): Use when deciding between ue-mcp's native category actions and Epic's wrapped ToolsetRegistry tools (the `epic_*` actions, incl. the Blueprint graph DSL) for a task in Unreal. Pulls in when authoring Blueprint graph bodies, or any time both a native action and an epic_* action could do the job and you need to pick.
- `ue-mcp-native-cpp` (`skill://ue-mcp/ue-mcp-native-cpp`): Use when writing or modifying native C++ UCLASSes in an Unreal project via ue-mcp. Covers create_cpp_class → write_cpp_file → live_coding_compile loop, when to use build vs Live Coding, and the add_module_dependency workflow. Pulls in any time the user asks to write a new native class, add a UPROPERTY, or implement a UFUNCTION.
- `ue-mcp-niagara` (`skill://ue-mcp/ue-mcp-niagara`): Use when authoring Niagara VFX systems via ue-mcp - creating systems and emitters, adding renderers, setting module inputs and static switches, building HLSL modules, and batching operations. Pulls in any time the user asks for a particle system, VFX, Niagara emitter, or motion matching cost visualization.
- `ue-mcp-workflow` (`skill://ue-mcp/ue-mcp-workflow`): Use when driving Unreal Engine editor via the ue-mcp MCP server. Covers the required order of operations (status check first), editor lifecycle, project scoping, and what to do when the bridge says "still initializing". Pulls in automatically any time the user asks to use an Unreal project or references ue-mcp tools.
<!-- ue-mcp:end -->
