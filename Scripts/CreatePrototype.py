"""Recreate missing prototype assets via UnrealEditor-Cmd -run=pythonscript -script=...
The checked-in Content assets normally make this unnecessary. Existing assets are preserved.
"""
import unreal

assets = unreal.AssetToolsHelpers.get_asset_tools()
unreal.EditorAssetLibrary.make_directory('/Game/Prototype')
unreal.EditorAssetLibrary.make_directory('/Game/Maps')

material_path = '/Game/Prototype/M_Prototype'
mat = unreal.load_asset(material_path)
if not mat:
    mat = assets.create_asset('M_Prototype', '/Game/Prototype', unreal.Material, unreal.MaterialFactoryNew())
    edit = unreal.MaterialEditingLibrary
    color = edit.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -450, 0)
    color.set_editor_property('parameter_name', 'Tint')
    color.set_editor_property('default_value', unreal.LinearColor(0.2, 0.4, 0.45, 1))
    edit.connect_material_property(color, '', unreal.MaterialProperty.MP_BASE_COLOR)
    glow = edit.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -450, 200)
    glow.set_editor_property('parameter_name', 'Glow')
    glow.set_editor_property('default_value', 0.0)
    multiply = edit.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, 180)
    edit.connect_material_expressions(color, '', multiply, 'A')
    edit.connect_material_expressions(glow, '', multiply, 'B')
    edit.connect_material_property(multiply, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    rough = edit.create_material_expression(mat, unreal.MaterialExpressionConstant, -200, 350)
    rough.set_editor_property('r', 0.8)
    edit.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)

if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/Archipelago'):
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Maps/Archipelago')
unreal.log('BORROWED GODS: prototype material and map created.')
