from collections import deque
from copy import deepcopy
from typing import Tuple
from dataclasses import dataclass, replace, field
from enum import Enum

def invert_pattern(pattern: str) -> str:
    """
    Inverts a sequence of balanced parantheses

    ((replace all parantheses to opposite) o (reverse))(pattern)
    
    Example:
        11 -> 22            & (( -> ))
        12 -> 12            & () -> ()
        122212 -> 121112    & ()))() -> ()((()
    """
    if all(symbol in ['1', '2'] for symbol in pattern):
        return ''.join('1' if symbol == '2' else '2' for symbol in pattern[::-1])

    if all(symbol in ['(', ')'] for symbol in pattern):
        return ''.join('(' if symbol == ')' else ')' for symbol in pattern[::-1])

    raise ValueError("invert_pattern: invalid pattern")

@dataclass()
class Block():
    class Type(Enum):
        STANDARD = ''
        EXTEND = 'e'
        UNITE = 'u'

        def __str__(self) -> str:
            return self.value

    __valid_patterns = {
        Type.STANDARD: ["11", "221", "222"],
        Type.EXTEND: ["12", "21"],
        Type.UNITE: ["11", "12", "21", "22"],
    }

    _type: Type
    _pattern: str

    def __post_init__(self):
        if not all(symbol in ['1', '2'] for symbol in self._pattern):
            raise ValueError("pattern contains invalid symbols")

        if not self._pattern in self.__valid_patterns[self._type]:
            raise ValueError("unrecognised block pattern")

    def __str__(self) -> str:
        return f'{self._type}{self._pattern}'

    @property
    def is_open(self) -> bool:
        return self._type == self.Type.STANDARD and self._pattern == "11"

    @property
    def is_closing(self) -> bool:
        return self._type == self.Type.STANDARD and self._pattern in ["221", "222"]
    
    @property
    def is_extend(self) -> bool:
        return self._type == self.Type.EXTEND
    
    @property
    def is_unite(self) -> bool:
        return self._type == self.Type.UNITE
    
    @property
    def is_anomaly(self) -> bool:
        return self.is_extend or self.is_unite

    def get_oc_pattern(self) -> str:
        if self.is_open:
            return '1'
        if self.is_closing:
            return '2'
        if self.is_extend:
            return '21'
        return ''


@dataclass
class Context:
    word: str
    blocks_a: list[Block] = field(default_factory=list)
    blocks_b: list[Block] = field(default_factory=list)

MAX_DEPTH = 20

@dataclass
class State():
    name: str
    final: bool = False
    anomaly: bool = False

@dataclass
class Transition:
    to: State
    letter: str
    block: Block | None = None

@dataclass
class NFA():
    _initial_state: State
    _transitions: dict[str, list[Transition]] = field(default_factory=dict)

    def get_initial(self) -> State:
        return self._initial_state
    
    def get_transitions(self, state: State) -> list[Transition]:
        return self._transitions[state.name]


def build_nfa(initial_state: State, base_transitions: dict[str, list[Transition]], 
              anomaly_pattern: str | None = None, anomaly_block: Block | None = None) -> NFA:
    
    s_initial_state = replace(
        initial_state,
        name=f's::{initial_state.name}', 
        anomaly=False
    )
    a_initial_state = replace(
        initial_state, 
        name=f'a::{initial_state.name}', 
        anomaly=True
    )
    s_transitions: dict[str, list[Transition]] = {}
    a_transitions: dict[str, list[Transition]] = {}

    for current_name, transitions in base_transitions.items():
        s_transitions[f's::{current_name}'] = []
        a_transitions[f'a::{current_name}'] = []
        for transition in transitions:
            s_transitions[f's::{current_name}'].append(replace(
                transition,
                to=replace(transition.to, name=f's::{transition.to.name}', anomaly=False),
            ))
            a_transitions[f'a::{current_name}'].append(replace(
                transition,
                to=replace(transition.to, name=f'a::{transition.to.name}', anomaly=True)
            ))
    
    if  (not (anomaly_block and anomaly_pattern and anomaly_block.is_anomaly)):
        return NFA(s_initial_state, s_transitions)

    junction = (
        Transition(
            to=State(name='juntion'),
            letter=anomaly_pattern[0],
        ), Transition(
            to=a_initial_state,
            letter=anomaly_pattern[1],
            block=anomaly_block
        )
    )

    merged = s_transitions | a_transitions
    merged[s_initial_state.name].append(junction[0])
    merged['juntion'] = [junction[1]]

    return NFA(s_initial_state, merged)

def nfa_intersection(nfa_a: NFA, nfa_b: NFA):
    """
    Perform a BFS traversal on decart product of two NFAs.
    Stops exploring further once both states are final.
    
    Args:
        nfa: Two NFA objects
        
    Yields:
        Tuple[list[Block], list[Block]]: description of a path in NFA product which brought to a final state
    """

    def update_context(transition_a: Transition, transition_b: Transition, ctx: Context) -> Context:
        letter = transition_a.letter
        new_ctx = deepcopy(ctx)
        new_ctx.word += letter 
        
        if transition_a.block:
            new_ctx.blocks_a = new_ctx.blocks_a + [transition_a.block]
        if transition_b.block:
            new_ctx.blocks_b = [transition_b.block] + new_ctx.blocks_b
        return new_ctx

    
    bfs_queue = deque[Tuple[State, State, Context]]()
    bfs_queue.append((
        nfa_a.get_initial(),
        nfa_b.get_initial(),
        Context("")
    ))
    
    while bfs_queue:
        state_a, state_b, ctx = bfs_queue.popleft()

        if len(ctx.word) > MAX_DEPTH:
            raise RuntimeError("max_depth reached")
        
        if state_a.final and state_b.final and len(ctx.word) > 0:
            yield (ctx.blocks_a, ctx.blocks_b)
            continue
        
        transitions_a = nfa_a.get_transitions(state_a)
        transitions_b = nfa_b.get_transitions(state_b)

        for transition_a in transitions_a:
            for transition_b in transitions_b:

                if transition_a.letter != transition_b.letter:
                    continue
                
                new_ctx = update_context(
                    transition_a=transition_a,
                    transition_b=transition_b,
                    ctx=ctx
                )
                bfs_queue.append((
                    transition_a.to,
                    transition_b.to,
                    new_ctx
                ))


states: dict[str, State] = {
    "forward::init":        State("forward::init", final=True),
    "forward::open":        State("forward::open"),
    "forward::closing_x":   State("forward::closing_x"),
    "forward::closing_y":   State("forward::closing_y"),

    "backward::init":       State("backward::init", final=True),
    "backward::open":       State("backward::open"),
    "backward::closing_x":  State("backward::closing_x"),
    "backward::closing_y":  State("backward::closing_y"),
}

transitions_forward: dict[str, list[Transition]] = {
    "forward::init": [
        Transition(to=states["forward::open"], letter="1"),
        Transition(to=states["forward::closing_x"], letter="2")
    ],
    "forward::open": [Transition(
        to=states["forward::init"], letter="1", 
        block=Block(Block.Type.STANDARD, "11")
    )],
    "forward::closing_x":   [Transition(to=states["forward::closing_y"], letter="2")],
    "forward::closing_y":   [
        Transition(
            to=states["forward::init"], letter="1", 
            block=Block(Block.Type.STANDARD, "221")
        ),
        Transition(
            to=states["forward::init"], letter="2",
            block=Block(Block.Type.STANDARD, "222")
        )
    ]
}

transitions_backward: dict[str, list[Transition]] = {
    "backward::init": [
        Transition(
            to=states["backward::open"], letter="2",
            block=Block(Block.Type.STANDARD, "11")
        ),
        Transition(
            to=states["backward::closing_x"], letter="2",
            block=Block(Block.Type.STANDARD, "221")
        ),
        Transition(
            to=states["backward::closing_x"], letter="1",
            block=Block(Block.Type.STANDARD, "222")
        )
    ],
    "backward::open":        [Transition(to=states["backward::init"],       letter="2")],
    "backward::closing_x":   [Transition(to=states["backward::closing_y"],  letter="1")],
    "backward::closing_y":   [Transition(to=states["backward::init"],       letter="1")]
}

def get_nfa_forward(anomaly_block: Block | None) -> NFA:
    anomaly_pattern = anomaly_block._pattern if anomaly_block else None
    return build_nfa(
        initial_state=states["forward::init"],
        base_transitions=transitions_forward,
        anomaly_pattern=anomaly_pattern,
        anomaly_block=anomaly_block
    )

def get_nfa_backward(anomaly_block: Block | None) -> NFA:
    anomaly_pattern = invert_pattern(anomaly_block._pattern) if anomaly_block else None
    return build_nfa(
        initial_state=states["backward::init"],
        base_transitions=transitions_backward,
        anomaly_pattern=anomaly_pattern,
        anomaly_block=anomaly_block
    )