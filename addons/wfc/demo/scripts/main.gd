extends Node2D

# Referencias a los nodos
@onready var wfc = $WFCGenerator2D
@onready var camera = $Camera2D

@onready var build_button = $DemoInterface/FullInterface/LeftSide/Buttons/BuildButton
@onready var run_button = $DemoInterface/FullInterface/LeftSide/Buttons/RunButton
@onready var build_model_text = $DemoInterface/FullInterface/LeftSide/BuildModelText
@onready var build_time_text = $DemoInterface/FullInterface/LeftSide/BuildTimeText
@onready var seed_text = $DemoInterface/FullInterface/LeftSide/SeedText
@onready var run_model_text = $DemoInterface/FullInterface/LeftSide/RunModelText
@onready var run_id_text = $DemoInterface/FullInterface/LeftSide/RunIDText
@onready var contradictions_text = $DemoInterface/FullInterface/LeftSide/ContradictionsText
@onready var run_time_text = $DemoInterface/FullInterface/LeftSide/RunTimeText

@onready var sample_button = $DemoInterface/FullInterface/RightSide/Right/TopRightButtons/LeftButtons/SampleButton
@onready var patterns_button = $DemoInterface/FullInterface/RightSide/Right/TopRightButtons/LeftButtons/PatternsButton
@onready var output_button = $DemoInterface/FullInterface/RightSide/Right/TopRightButtons/OutputButton

@onready var sample_map = $DemoInterface/FullInterface/RightSide/Right/BotRightMaps/Layers/SampleMap
@onready var patterns = $DemoInterface/FullInterface/RightSide/Right/BotRightMaps/Layers/Patterns
@onready var output_map = $DemoInterface/FullInterface/RightSide/Right/BotRightMaps/Layers/OutputMap

# Variables para controlar el desplazamiento de la cámara
var camera_speed = 300  # Velocidad de movimiento en píxeles por segundo

# Variables para los botones
var building = false
var building_time = 0
var running = false
var running_time = 0
var next_run_id = 0
var run_success = true

# Called when the node enters the scene tree for the first time.
func _ready():
	# Conectar señales de botones
	build_button.pressed.connect(self._on_build_button_pressed)
	run_button.pressed.connect(self._on_run_button_pressed)
	
	sample_button.pressed.connect(self._on_sample_map_button_pressed)
	patterns_button.pressed.connect(self._on_patterns_button_pressed)
	output_button.pressed.connect(self._on_output_map_button_pressed)

func _on_build_button_pressed():
	if not building and not running:
		building = true
		build_model_text.text = "The model is being built."
		#patterns_button.visible = wfc.get_paint_patterns()
		var start_time = Time.get_ticks_usec()
		var success = await wfc.build_model()
		building_time = (Time.get_ticks_usec() - start_time) / 1000.0
		build_time_text.text = "Build time: %.2f ms" % building_time
		if success:
			build_model_text.text = "The model has been built."
		else:
			build_model_text.text = "The model has NOT been built."
		seed_text.text = "Seed used: %.d" % wfc.get_execution_seed()
		building = false

func _on_run_button_pressed():
	if not building and not running:
		running = true
		run_model_text.text = "The model has started running."
		run_id_text.text = "Run ID: %.d" % next_run_id
		next_run_id += 1
		running_time = Time.get_ticks_usec()
		wfc.run_model()

func _on_wfc_finished_running():
	wfc.finish_thread()
	running_time = (Time.get_ticks_usec() - running_time) / 1000.0
	run_time_text.text = "Run time: %.2f ms" % running_time
	if run_success:
		run_model_text.text = "The model has finished running."
	else:
		run_model_text.text = "The model has NOT finished running."
	contradictions_text.text = "Contradictions: %.d" % wfc.get_pattern_size()
	running = false

func _on_sample_map_button_pressed():
	sample_map.visible = true
	patterns.visible = false
	output_map.visible = false

func _on_patterns_button_pressed():
	sample_map.visible = false
	patterns.visible = true
	output_map.visible = false

func _on_output_map_button_pressed():
	sample_map.visible = false
	patterns.visible = false
	output_map.visible = true

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	_handle_camera_movement(delta)

func _handle_camera_movement(delta):
	# Movimiento básico con las flechas del teclado
	var movement = Vector2.ZERO
	
	if Input.is_action_pressed("ui_left"):
		movement.x -= 1
	if Input.is_action_pressed("ui_right"):
		movement.x += 1
	if Input.is_action_pressed("ui_up"):
		movement.y -= 1
	if Input.is_action_pressed("ui_down"):
		movement.y += 1
	
	# Normalizar el vector de movimiento y ajustar con la velocidad y el delta
	if movement != Vector2.ZERO:
		camera.position += movement.normalized() * camera_speed * delta
