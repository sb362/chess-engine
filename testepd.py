#!/usr/bin/env python3

import argparse
import pathlib

import chess
import chess.engine

parser = argparse.ArgumentParser()

parser.add_argument("epd", metavar="path", type=str, nargs='+', help="One or more .epd files")
parser.add_argument("-e", "--engine", metavar="path", type=str,
					default="build/src/main.exe", help="Engine path")

args = parser.parse_args()

epd_paths = [pathlib.Path(s) for s in args.epd]
engine = chess.engine.SimpleEngine.popen_uci(args.engine)

limits = chess.engine.Limit(time=2)

for epd_path in epd_paths:
	with open(epd_path, 'r') as f:
		lines = list(filter(lambda s: s and not s.startswith('#'), (line.strip() for line in f)))

	print(f"{epd_path.stem} ({len(lines)} EPDs)")

	correct = 0

	board = chess.Board()
	for line in lines:
		epd_info = board.set_epd(line)

		assert "id" in epd_info
		assert "bm" in epd_info

		epd_id = epd_info["id"]
		best_moves = epd_info["bm"]

		print(f"{epd_id:<12} {' or '.join(board.san(move) for move in best_moves): <10}", end='')

		result = engine.analyse(board, limits, info=chess.engine.INFO_PV | chess.engine.INFO_SCORE)
		engine_move = result["pv"][0]
		engine_score = result["score"].relative
		engine_depth = result["depth"]
		engine_seldepth = result["seldepth"]

		if engine_move in best_moves:
			correct += 1
			print(f"{board.san(engine_move): <5} (correct)   {engine_depth:>2}/{engine_seldepth:<2} {engine_score}")
		else:
			print(f"{board.san(engine_move): <5} (incorrect) {engine_depth:>2}/{engine_seldepth:<2} {engine_score}")

	print(f"{correct}/{len(lines)} ({(correct / len(lines)):.0%}) correct")

engine.quit()
