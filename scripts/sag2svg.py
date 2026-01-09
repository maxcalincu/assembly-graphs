#!/usr/bin/env python3
import math
from collections import deque, defaultdict
import svgwrite

import argparse
import sys
import random

def sag2svg(word: list, output_file: str) -> None:
    """
    Creates an SVG image of a graph from a double-occurrence word.
    
    Args:
        word: A string where each letter appears exactly twice
        output_file: Path to save the SVG file
    """
    # Constants
    EPS = 1e-5              # Floating point precision
    L = 0.45                # Length of control point vectors
    SCALE = 150
    NODE_RADIUS = 4         
    STROKE_WIDTH = 0.5
    DISTORTION_X = 0.08
    DISTORTION_Y = 0.28

    def approx_equal(x, y):
        return abs(x[0] - y[0]) + abs(x[1] - y[1]) < EPS

    def L1(point_a, point_b):
        return abs(point_a[0] - point_b[0]) + abs(point_a[1] - point_b[1])

    def L2(point_a, point_b):
        return (point_a[0] - point_b[0])*(point_a[0] - point_b[0]) + (point_a[1] - point_b[1])*(point_a[1] - point_b[1])

    # Step 1: Parse the word and build graph adjacency
    letters = list(set(word))
    n = len(letters)
    
    # word is expected to be a double-occurence word
    if any(word.count(letter) != 2 for letter in letters):
        raise ValueError("an invalid double-occurance word was passed")

    adjacency = defaultdict(set)
    for i in range(len(word) - 1):
        adjacency[word[i    ]].add(word[i + 1])
        adjacency[word[i + 1]].add(word[i    ])
    
    # Step 2: BFS to assign vertex coordinates
    visited = [word[0]]
    vertex_coordinates = {word[0]: (0.0, 0.0)}
    queue = deque([word[0]])
    
    def get_unoccupied_points(vertex):
        """Get 4 unoccupied integer neighbouring ``vertex`` in L1 distance order"""
        x, y = vertex_coordinates[vertex]
        candidates = []
        dist = 1
        while len(candidates) < 4:
            for dx in range(0, dist + 1):
                dy = dist - dx
                signs = [(1, 1), (1, -1), (-1, 1), (-1, -1)]
                if dx == 0:
                    signs = [(1, 1), (1, -1)]
                if dy == 0:
                    signs = [(1, 1), (-1, 1)]
                for sx, sy in signs:
                    point = (x + SCALE*sx*dx, y + SCALE*sy*dy)
                    if not any(approx_equal(point, coord) for coord in vertex_coordinates.values()):
                        candidates.append(point)       
            dist += 1

        return candidates[:4]

    while len(queue) > 0:
        current = queue.popleft()
        candidates = get_unoccupied_points(vertex=current)
        random.shuffle(candidates)
        candidates = sorted(candidates, key=lambda candidate: int(1e10)*(L1(candidate, vertex_coordinates[current])) + L2(candidate, (0, 0)))
        for neighbor in adjacency[current]:
            if neighbor not in visited:
                vertex_coordinates[neighbor] = candidates.pop(0)
                visited.append(neighbor)
                queue.append(neighbor)
    
    for vertex, (x, y) in vertex_coordinates.items():
        if int(abs(y)/SCALE + EPS)%2 == 1:
            y += SCALE * DISTORTION_Y
            vertex_coordinates[vertex] = (x, y)
        if int(abs(x)/SCALE + EPS)%2 == 1:
            x += SCALE * DISTORTION_X
            vertex_coordinates[vertex] = (x, y)

    # Step 3: Define allowed directions (angles)
    # Solutions of (z^12 - 1)/(z^4 - 1) = 0
    # These are 12th roots of unity excluding 4th roots (0°, 90°, 180°, 270°)
    allowed_directions = []
    for angle_degrees in [30 * index for index in range(12) if (4*index) % 12]: #[30, 60, 120, 150, 210, 240, 300, 330]
        angle = math.radians(angle_degrees) 
        allowed_directions.append((L * SCALE * math.sin(angle), L * SCALE * math.cos(angle)))
    
    # Step 4: Determine the control points of cubic bezier curves

    # A cubic bezier curve is defined by 4 points
    # P0 and P3 are the endpoints of an edge
    # vector P1 - P0 is colinear with the derivative of the curve in P0
    # vector P2 - P3 is colinear with the derivative of the curve in P3  

    start_control_point = {}    #P1
    end_control_point = {}      #P2

        
    def get_unoccupied_directions(vertex):
        occupied_control_points = list(start_control_point.values()) + list(end_control_point.values())

        def is_occupied(direction):
            point_a = tuple(v + d for d, v in zip(direction, vertex_coordinates[vertex])) # vertex + direction
            point_b = tuple(v - d for d, v in zip(direction, vertex_coordinates[vertex])) # vertex - direction
            return any([
                any(approx_equal(point_a, control_point) for control_point in occupied_control_points),
                any(approx_equal(point_b, control_point) for control_point in occupied_control_points)   
            ])

        return [direction for direction in allowed_directions if not is_occupied(direction)]
    
    def best_direction(vectors, candidates):
        def scalar_product(point_a, point_b):
            x_a, y_a = point_a
            x_b, y_b = point_b
            return x_a * x_b + y_a * y_b
        
        def key(candidate):
            if len(vectors) == 1:
                return scalar_product(vectors[0], candidate)
            return min(scalar_product(vectors[0], candidate), - scalar_product(vectors[1], candidate))

        return max(candidates, key=key)
    
    def get_symmetric(point, vector):
        return tuple(2*p - v for p, v in zip(point, vector))

    def get_vectors(vertex, indexes):
        return list(map(
            lambda v: tuple(x - cur for x, cur in zip(v, vertex_coordinates[word[vertex]])),
            [vertex_coordinates[word[index]] for index in indexes if index < len(word) and index >= 0]
        ))

    for i in range(0, len(word) - 1):
        start_control_point[i] = get_symmetric(point=vertex_coordinates[word[i]], vector=end_control_point[i - 1]) if i - 1 in end_control_point else tuple(
            x + y for x, y in zip(
                vertex_coordinates[word[i]],
                best_direction(
                    get_vectors(i, [i + 1, i - 1]),
                    get_unoccupied_directions(vertex=word[i])
                )
            )
        )
        
        end_control_point[i] = get_symmetric(point=vertex_coordinates[word[i + 1]], vector=start_control_point[i + 1]) if i + 1 in start_control_point else tuple(
            x + y for x, y in zip(
                vertex_coordinates[word[i + 1]], 
                best_direction(
                    get_vectors(i + 1, [i, i + 2]),
                    get_unoccupied_directions(vertex=word[i + 1])
                )
            )
        )

    cubic_bezier_curves = [(vertex_coordinates[word[i]], start_control_point[i], end_control_point[i], vertex_coordinates[word[i + 1]]) for i in range(0, len(word) - 1)]

    # Step 5: Create SVG
    # Calculate bounding box
    all_points = []
    for coord in vertex_coordinates.values():
        all_points.append(coord)
    for p0, p1, p2, p3 in cubic_bezier_curves:
        all_points.extend([p0, p1, p2, p3])
    
    xs = [p[0] for p in all_points]
    ys = [p[1] for p in all_points]
    
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    
    # Add padding
    padding = 100
    width = max_x - min_x + 2 * padding
    height = max_y - min_y + 2 * padding
    
    # Create SVG drawing
    dwg = svgwrite.Drawing(output_file, size=(width, height), profile='full')
    dwg.add(dwg.rect(insert=(0, 0), size=('100%', '100%'), fill='white'))

    # Define a group for the graph
    graph_group = dwg.g(id='graph')
    
    # Draw edges first (so they appear behind nodes)
    for i, (p0, p1, p2, p3) in enumerate(cubic_bezier_curves):
        # Adjust coordinates with padding
        adj_p0 = (p0[0] - min_x + padding, p0[1] - min_y + padding)
        adj_p1 = (p1[0] - min_x + padding, p1[1] - min_y + padding)
        adj_p2 = (p2[0] - min_x + padding, p2[1] - min_y + padding)
        adj_p3 = (p3[0] - min_x + padding, p3[1] - min_y + padding)

        # Create cubic Bezier path
        path = dwg.path(d=f"M {adj_p0[0]},{adj_p0[1]} "
                         f"C {adj_p1[0]},{adj_p1[1]} "
                         f"{adj_p2[0]},{adj_p2[1]} "
                         f"{adj_p3[0]},{adj_p3[1]}",
                       fill='none',
                       stroke=f'black',
                       stroke_width=STROKE_WIDTH)
        graph_group.add(path)
    
    # Draw vertices
    for letter, (x, y) in vertex_coordinates.items():
        adj_x = x - min_x + padding
        adj_y = y - min_y + padding
        
        # Draw circle for vertex
        circle = dwg.circle(center=(adj_x, adj_y),
                           r=NODE_RADIUS,
                           fill='white' if letter != word[0] and letter != word[2 * n - 1] else 'red',
                           stroke='black',
                           stroke_width=STROKE_WIDTH)
        graph_group.add(circle)
        
        # Add letter label
        text = dwg.text(letter,
                       insert=(adj_x, adj_y),
                       text_anchor='middle',
                       dominant_baseline='middle',
                       font_size=NODE_RADIUS * 1.3,
                       font_weight='bold')
        graph_group.add(text)
    
    # Add the graph group to the drawing
    dwg.add(graph_group)
    
    # Save the SVG file
    dwg.save()    
    return


def main():
    parser = argparse.ArgumentParser(
        description='Create an SVG for a simple assembly graph',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument(
        '--input',
        '-i',
        type=str,
        required=True,
        help='Input string to process'
    )
    parser.add_argument(
        '--output',
        '-o',
        type=str,
        required=False,
        default="graph.svg",
        help='Output file name'
    )
    args = parser.parse_args()
    
    try:
        input = [int(vertex) for vertex in args.input.split(",")]
        if not args.output.endswith(".svg"):
            raise ValueError("output file should be *.svg")
        sag2svg(input, args.output)
        print(f"File {args.output} was successfuly generated with {input}")

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()