extends Node2D


const TILE_SIZE: int = 75
const BOARD_PX: int = TILE_SIZE * 8  # 600px


const LIGHT_COLOR = Color(0.941, 0.851, 0.710)    
const DARK_COLOR = Color(0.710, 0.533, 0.388)      
const SELECT_COLOR = Color(1.0, 1.0, 0.0, 0.45)      
const LEGAL_DOT_COLOR = Color(0.0, 0.0, 0.0, 0.25)   
const LEGAL_RING_COLOR = Color(0.0, 0.0, 0.0, 0.25)
const LAST_MOVE_COLOR = Color(0.667, 0.843, 0.318, 0.5) 
const CHECK_COLOR = Color(1.0, 0.0, 0.0, 0.5)         


signal move_attempt(move_str: String)


var board_data: Array = []      
var is_flipped: bool = false
var selected_sq: Vector2i = Vector2i(-1, -1) 
var legal_moves: PackedStringArray = PackedStringArray()
var filtered_moves: PackedStringArray = PackedStringArray()  
var last_move_str: String = ""
var check_sq: Vector2i = Vector2i(-1, -1)  
var interactive: bool = true               


var piece_textures: Dictionary = {}


var piece_sprites: Array = []


func _ready():
	_load_textures()
	_create_sprite_grid()
	queue_redraw()

func _load_textures():
	var mapping = {
		"P": "res://Icons/pawn_white.png",
		"N": "res://Icons/knight_white.png",
		"B": "res://Icons/bishop_white.png",
		"R": "res://Icons/rook_white.png",
		"Q": "res://Icons/queen_white.png",
		"K": "res://Icons/king_white.png",
		"p": "res://Icons/pawn.png",
		"n": "res://Icons/knight.png",
		"b": "res://Icons/bishop.png",
		"r": "res://Icons/rook.png",
		"q": "res://Icons/queen.png",
		"k": "res://Icons/king.png",
	}
	for code in mapping:
		if ResourceLoader.exists(mapping[code]):
			piece_textures[code] = load(mapping[code])
		else:
			push_warning("Missing piece texture: " + mapping[code])

func _create_sprite_grid():

	for child in get_children():
		if child is Sprite2D:
			child.queue_free()
	
	piece_sprites = []
	for row in range(8):
		var row_arr: Array = []
		for col in range(8):
			var sprite = Sprite2D.new()
			sprite.z_index = 2
			sprite.visible = false
			add_child(sprite)
			row_arr.append(sprite)
		piece_sprites.append(row_arr)


func _draw():

	for row in range(8):
		for col in range(8):
			var rect = Rect2(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE)

			var data_row: int
			var data_col: int
			if not is_flipped:
				data_row = row
				data_col = col
			else:
				data_row = 7 - row
				data_col = 7 - col
			
			var is_light = (data_row + data_col) % 2 == 1
			draw_rect(rect, LIGHT_COLOR if is_light else DARK_COLOR)
	

	if last_move_str.length() >= 4:
		var from_disp = _data_to_display(_move_str_to_data(last_move_str.substr(0, 2)))
		var to_disp = _data_to_display(_move_str_to_data(last_move_str.substr(2, 2)))
		if from_disp.x >= 0:
			draw_rect(Rect2(from_disp.x * TILE_SIZE, from_disp.y * TILE_SIZE, TILE_SIZE, TILE_SIZE), LAST_MOVE_COLOR)
		if to_disp.x >= 0:
			draw_rect(Rect2(to_disp.x * TILE_SIZE, to_disp.y * TILE_SIZE, TILE_SIZE, TILE_SIZE), LAST_MOVE_COLOR)
	

	if check_sq != Vector2i(-1, -1):
		var disp = _data_to_display(check_sq)
		draw_rect(Rect2(disp.x * TILE_SIZE, disp.y * TILE_SIZE, TILE_SIZE, TILE_SIZE), CHECK_COLOR)
	

	if selected_sq != Vector2i(-1, -1):
		var disp = _data_to_display(selected_sq)
		draw_rect(Rect2(disp.x * TILE_SIZE, disp.y * TILE_SIZE, TILE_SIZE, TILE_SIZE), SELECT_COLOR)
	

	for move_str in filtered_moves:
		var target = _move_str_to_data(move_str.substr(2, 2))
		var disp = _data_to_display(target)
		var center = Vector2(disp.x * TILE_SIZE + TILE_SIZE / 2.0, disp.y * TILE_SIZE + TILE_SIZE / 2.0)
		

		var piece_at_target = _get_piece_at_data(target)
		if piece_at_target != ".":
			draw_arc(center, TILE_SIZE * 0.42, 0, TAU, 32, LEGAL_RING_COLOR, 5.0)
		else:
			draw_circle(center, TILE_SIZE * 0.14, LEGAL_DOT_COLOR)
	

	var font = ThemeDB.fallback_font
	var font_size = 11
	for i in range(8):

		var file_idx = i if not is_flipped else 7 - i
		var file_char = char("a".unicode_at(0) + file_idx)
		var label_color = DARK_COLOR if (7 + i) % 2 == 1 else LIGHT_COLOR
		draw_string(font, Vector2(i * TILE_SIZE + TILE_SIZE - 12, BOARD_PX - 4), file_char, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, label_color)
		

		var rank_idx: int
		if not is_flipped:
			rank_idx = 8 - i
		else:
			rank_idx = i + 1
		var rank_str = str(rank_idx)
		var rlabel_color = LIGHT_COLOR if (0 + i) % 2 == 1 else DARK_COLOR
		draw_string(font, Vector2(3, i * TILE_SIZE + 14), rank_str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, rlabel_color)
	

	draw_rect(Rect2(0, 0, BOARD_PX, BOARD_PX), Color(0.2, 0.15, 0.1), false, 2.0)


func update_board(rows: Array):
	board_data = rows
	_update_sprites()
	queue_redraw()

func _update_sprites():
	if board_data.is_empty() or piece_sprites.is_empty():
		return
	
	for display_row in range(8):
		for display_col in range(8):
			var data_row: int
			var data_col: int
			if not is_flipped:
				data_row = display_row
				data_col = display_col
			else:
				data_row = 7 - display_row
				data_col = 7 - display_col
			
			var sprite: Sprite2D = piece_sprites[display_row][display_col]
			var piece_code = "."
			if data_row < board_data.size() and data_col < board_data[data_row].size():
				piece_code = board_data[data_row][data_col]
			
			if piece_code != "." and piece_code in piece_textures:
				sprite.texture = piece_textures[piece_code]
				sprite.position = Vector2(
					display_col * TILE_SIZE + TILE_SIZE / 2.0,
					display_row * TILE_SIZE + TILE_SIZE / 2.0
				)
				# Scale to fit
				var tex_size = sprite.texture.get_size()
				var scale_factor = (TILE_SIZE * 0.85) / max(tex_size.x, tex_size.y)
				sprite.scale = Vector2(scale_factor, scale_factor)
				sprite.visible = true
			else:
				sprite.visible = false


func _input(event):
	if not interactive:
		return
	
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		var local_pos = to_local(event.global_position)

		if local_pos.x < 0 or local_pos.x >= BOARD_PX or local_pos.y < 0 or local_pos.y >= BOARD_PX:
			return
		
		var display_col = int(local_pos.x / TILE_SIZE)
		var display_row = int(local_pos.y / TILE_SIZE)
		var data_pos = _display_to_data(Vector2i(display_col, display_row))
		
		_on_square_clicked(data_pos)

func _on_square_clicked(data_pos: Vector2i):
	if selected_sq == Vector2i(-1, -1):

		var piece = _get_piece_at_data(data_pos)
		if piece != ".":
			selected_sq = data_pos
			_filter_legal_moves()
			queue_redraw()
	else:

		var move_str = _data_to_move_str(selected_sq) + _data_to_move_str(data_pos)
		
		if move_str in filtered_moves:

			selected_sq = Vector2i(-1, -1)
			filtered_moves = PackedStringArray()
			queue_redraw()
			move_attempt.emit(move_str)
		else:

			var piece = _get_piece_at_data(data_pos)
			if piece != ".":
				selected_sq = data_pos
				_filter_legal_moves()
			else:
				selected_sq = Vector2i(-1, -1)
				filtered_moves = PackedStringArray()
			queue_redraw()

func _filter_legal_moves():
	filtered_moves = PackedStringArray()
	if selected_sq == Vector2i(-1, -1):
		return
	var from_str = _data_to_move_str(selected_sq)
	for m in legal_moves:
		if m.begins_with(from_str):
			filtered_moves.append(m)


func set_legal_moves(moves: PackedStringArray):
	legal_moves = moves
	if selected_sq != Vector2i(-1, -1):
		_filter_legal_moves()
	queue_redraw()

func set_last_move(move_str: String):
	last_move_str = move_str
	queue_redraw()

func set_check(data_pos: Vector2i):
	check_sq = data_pos
	queue_redraw()

func clear_check():
	check_sq = Vector2i(-1, -1)
	queue_redraw()

func clear_selection():
	selected_sq = Vector2i(-1, -1)
	filtered_moves = PackedStringArray()
	queue_redraw()




func _move_str_to_data(sq_str: String) -> Vector2i:
	if sq_str.length() < 2:
		return Vector2i(-1, -1)
	var file = sq_str.unicode_at(0) - "a".unicode_at(0)  
	var rank = int(sq_str.substr(1, 1))                  
	var data_row = 8 - rank  
	return Vector2i(file, data_row)


func _data_to_move_str(data_pos: Vector2i) -> String:
	var file_char = char("a".unicode_at(0) + data_pos.x)
	var rank = 8 - data_pos.y 
	return file_char + str(rank)


func _data_to_display(data_pos: Vector2i) -> Vector2i:
	if not is_flipped:
		return data_pos
	else:
		return Vector2i(7 - data_pos.x, 7 - data_pos.y)

func _display_to_data(disp_pos: Vector2i) -> Vector2i:
	if not is_flipped:
		return disp_pos
	else:
		return Vector2i(7 - disp_pos.x, 7 - disp_pos.y)


func _get_piece_at_data(data_pos: Vector2i) -> String:
	if board_data.is_empty():
		return "."
	if data_pos.y >= 0 and data_pos.y < 8 and data_pos.x >= 0 and data_pos.x < 8:
		if data_pos.y < board_data.size() and data_pos.x < board_data[data_pos.y].size():
			return board_data[data_pos.y][data_pos.x]
	return "."


func find_king(code: String) -> Vector2i:
	for row in range(8):
		if row >= board_data.size():
			continue
		for col in range(8):
			if col >= board_data[row].size():
				continue
			if board_data[row][col] == code:
				return Vector2i(col, row)
	return Vector2i(-1, -1)
