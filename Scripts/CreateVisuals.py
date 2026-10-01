"""Create versioned procedural surface materials; preserve existing assets."""
import unreal

assets = unreal.AssetToolsHelpers.get_asset_tools()
edit = unreal.MaterialEditingLibrary

def create(name):
    path = '/Game/Prototype/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return None
    return assets.create_asset(name, '/Game/Prototype', unreal.Material, unreal.MaterialFactoryNew())

def node(mat, cls, **properties):
    result = edit.create_material_expression(mat, cls)
    for key, value in properties.items():
        result.set_editor_property(key, value)
    return result

def wire(source, target, pin, output=''):
    if not edit.connect_material_expressions(source, output, target, pin):
        raise RuntimeError('Material connection failed: ' + target.get_name() + ' / ' + pin)

def scalar(mat, name, default):
    return node(mat, unreal.MaterialExpressionScalarParameter, parameter_name=name, default_value=default)

def constant(mat, value):
    return node(mat, unreal.MaterialExpressionConstant, r=value)

def multiply(mat, a, b):
    result = node(mat, unreal.MaterialExpressionMultiply)
    wire(a, result, 'A'); wire(b, result, 'B')
    return result

def add(mat, a, b):
    result = node(mat, unreal.MaterialExpressionAdd)
    wire(a, result, 'A'); wire(b, result, 'B')
    return result

mat = create('M_SanctuaryStone_v2')
if mat:
    color = node(mat, unreal.MaterialExpressionVectorParameter, parameter_name='Tint', default_value=unreal.LinearColor(.2, .3, .33, 1))
    world = node(mat, unreal.MaterialExpressionWorldPosition)
    noise = node(mat, unreal.MaterialExpressionNoise, scale=.008, quality=1, levels=2)
    wire(world, noise, '')
    variation = add(mat, multiply(mat, noise, constant(mat, .045)), constant(mat, .955))
    edit.connect_material_property(multiply(mat, color, variation), '', unreal.MaterialProperty.MP_BASE_COLOR)
    edit.connect_material_property(multiply(mat, color, scalar(mat, 'Glow', 0)), '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.connect_material_property(scalar(mat, 'Roughness', .82), '', unreal.MaterialProperty.MP_ROUGHNESS)
    edit.connect_material_property(scalar(mat, 'Metallic', 0), '', unreal.MaterialProperty.MP_METALLIC)
    edit.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)

mat = create('M_AegeanWater_v3')
if mat:
    # Two crossing world-space ripple fields. No downloaded textures or transparency.
    mat.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    color = node(mat, unreal.MaterialExpressionVectorParameter, parameter_name='Tint', default_value=unreal.LinearColor(.018, .13, .16, 1))
    world = node(mat, unreal.MaterialExpressionWorldPosition)
    x = node(mat, unreal.MaterialExpressionComponentMask, r=True, g=False, b=False, a=False)
    y = node(mat, unreal.MaterialExpressionComponentMask, r=False, g=True, b=False, a=False)
    wire(world, x, ''); wire(world, y, '')
    time = node(mat, unreal.MaterialExpressionTime)
    waves = []
    for axis, freq, speed in [(x, .005, .08), (y, .008, -.05)]:
        phase = add(mat, multiply(mat, axis, constant(mat, freq)), multiply(mat, time, constant(mat, speed)))
        wave = node(mat, unreal.MaterialExpressionSine)
        wire(phase, wave, '')
        waves.append(wave)
    ripples = add(mat, multiply(mat, add(mat, waves[0], waves[1]), constant(mat, .13)), constant(mat, .78))
    edit.connect_material_property(multiply(mat, color, ripples), '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)

# Persist the vertex-factory usage needed by the instanced bronze frieze.
stone = unreal.load_asset('/Game/Prototype/M_SanctuaryStone_v2')
stone.set_editor_property('used_with_instanced_static_meshes', True)
edit.recompile_material(stone)
unreal.EditorAssetLibrary.save_loaded_asset(stone)
unreal.log('BORROWED GODS: sanctuary stone and Aegean water ready.')
