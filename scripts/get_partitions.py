#!/usr/bin/env python3

from dataclasses import dataclass
from typing import Tuple
from nfa import Block, nfa_intersection, get_nfa_forward, get_nfa_backward, invert_pattern  
import json

@dataclass
class Partition:
    forward:  list[Block]
    backward: list[Block]

    def _get_anomaly(self, blocks: list[Block]) -> Block | None:
        anomalies = [block for block in blocks if block.is_extend or block.is_unite]
        assert(len(anomalies) < 2)
        return anomalies[0] if len(anomalies) > 0 else None

    def __str__(self) -> str:
        return ''.join(block._pattern for block in self.forward)
    
    def __add__(self, other):
        return Partition(
            forward=self.forward + other.forward, 
            backward=other.backward + self.backward
        )

    @property
    def is_tt(self):
        return self._get_anomaly(self.forward) and self._get_anomaly(self.backward)

    @property
    def is_tf(self):
        return self._get_anomaly(self.forward) and not self._get_anomaly(self.backward)
    
    @property
    def is_ft(self):
        return not self._get_anomaly(self.forward) and self._get_anomaly(self.backward)
    
    @property
    def is_ff(self):
        return not self._get_anomaly(self.forward) and not self._get_anomaly(self.backward)
    
    def get_oc_pattern(self) -> Tuple[str, str]:
        return (
            ''.join(block.get_oc_pattern() for block in self.forward),
            ''.join(block.get_oc_pattern() for block in self.backward)
        )
    
    def get_odd_z_pattern(self) -> Tuple[str, str]:
        def get_z_positions(blocks: list[Block]) -> list[int]:
            result = []
            location = 0
            for block in blocks:
                if block.is_closing:
                    result.append(location + 2)
                if block.is_unite:
                    result.append(location + 0)
                    result.append(location + 1)
                location += len(block._pattern)
            return result

        forward_positions =  get_z_positions(self.forward)
        backward_positions = get_z_positions(self.backward)

        forward_word =  ''.join(block._pattern for block in self.forward)
        backward_word = ''.join(block._pattern for block in self.backward)

        n = len(forward_word)
        return (
            ''.join(forward_word[pos]  for pos in forward_positions  if n - 1 - pos not in backward_positions),
            ''.join(backward_word[pos] for pos in backward_positions if n - 1 - pos not in forward_positions)
        )

def get_partitions(
        anomaly_forward: Block | None = None, 
        anomaly_backward: Block | None = None):

    return [Partition(blocks_forward, blocks_backward) for blocks_forward, blocks_backward in 
    nfa_intersection(
        nfa_a=get_nfa_forward(anomaly_forward),
        nfa_b=get_nfa_backward(anomaly_backward),
    )]

def min_depth(pattern: str) -> int:
    return min(i - 2 * pattern[:i].count('2') for i in range(len(pattern) + 1))      

def check_anomaly_partition(anomaly: Partition) -> bool:
    odd_z_forward, odd_z_backward = anomaly.get_odd_z_pattern()
    if min_depth(odd_z_forward) < 0 or min_depth(odd_z_backward) < 0:
        return False

    z_height_forward =  odd_z_forward.count('1') -  odd_z_forward.count('2')
    z_height_backward = odd_z_backward.count('1') - odd_z_backward.count('2')
    
    if z_height_forward%2 or z_height_backward%2:
        return False

    chunks_behind = z_height_backward//2
    chunks_after  = z_height_forward//2

    oc_forward, oc_backward = anomaly.get_oc_pattern()
    oc_height_forward =  oc_forward.count('1')  - oc_forward.count('2')
    oc_height_backward = oc_backward.count('1') - oc_backward.count('2')

    return (
        oc_height_forward  == 2 * chunks_after - 3 * chunks_behind and
        oc_height_backward == 2 * chunks_behind - 3 * chunks_after and

        min_depth(oc_forward) + 3 * chunks_behind + 1 >= 0 and
        min_depth(oc_backward) + 3 * chunks_after + 1 >= 0
    )


if __name__ == "__main__":

    anomalies_forward = [
        Block(Block.Type.EXTEND, "12"),
        Block(Block.Type.EXTEND, "21"),

        Block(Block.Type.UNITE, "11"),
        Block(Block.Type.UNITE, "12"),
        Block(Block.Type.UNITE, "21"),
        Block(Block.Type.UNITE, "22"),
    ]

    anomalies_backward = [
        Block(Block.Type.EXTEND, "12"),
        Block(Block.Type.EXTEND, "21"),

        Block(Block.Type.UNITE, "11"),
        Block(Block.Type.UNITE, "12"),
        Block(Block.Type.UNITE, "21"),
        Block(Block.Type.UNITE, "22"),
    ]

    data: dict[str, dict[str, list]] = {}

    for anomaly_forward in anomalies_forward:
        for anomaly_backward in anomalies_backward:
            partitions = get_partitions(
                anomaly_forward=anomaly_forward,
                anomaly_backward=anomaly_backward
            )
            tt = [partition for partition in partitions if partition.is_tt]
            tf = [partition for partition in partitions if partition.is_tf]
            ft = [partition for partition in partitions if partition.is_ft]
            ff = [partition for partition in partitions if partition.is_ff]

            key = f'{anomaly_forward}:{anomaly_backward}'
            data[key] = {
                "tt": [],
                "tf_ft": [],
                "ft_tf": [],
            }

            def check(partition: Partition, partition_type: str) -> bool:
                if check_anomaly_partition(partition):
                    data[key][partition_type].append({
                        "+": str(partition),
                        "-": invert_pattern(str(partition))
                    })
                    return True
                return False

            for partition in tt:
                check(partition=partition, partition_type='tt')

            for partition_tf in tf:
                for partition_ft in ft:
                    check(partition=partition_tf + partition_ft, partition_type="tf_ft")
                    check(partition=partition_ft + partition_tf, partition_type="ft_tf")
    
    with open("partitions.json", "w") as f:
        json.dump(data, f, sort_keys=True, indent=4)
