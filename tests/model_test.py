"""Reproducible public-API tests against a separate vertex/edge-set model."""

import ctypes as C
import random

I = C.c_int
L = C.c_long
S = C.c_ssize_t
P = C.c_char_p
B = C.c_bool
SIGNATURES = {
    'new_collection': (L, []),
    'delete_collection': (None, [L]),
    'size': (S, [L]),
    'new_graph': (B, [L, P, I]),
    'delete_graph': (None, [L, P]),
    'graph_new_node': (I, [L, P]),
    'graph_delete_node': (B, [L, P, I]),
    'graph_add_edge': (B, [L, P, I, I]),
    'graph_delete_edge': (B, [L, P, I, I]),
    'first_graph': (P, [L]),
    'next_graph': (P, [L, P]),
    'first_node': (I, [L, P]),
    'next_node': (I, [L, P, I]),
    'first_edge': (I, [L, P, I]),
    'next_edge': (I, [L, P, I, I]),
    'edge_level': (S, [L, P, I, I]),
    'graph_level': (S, [L, P]),
    'max_level': (S, []),
    'count_nodes': (S, [L, P]),
    'count_edges': (S, [L, P]),
    'replace_edge': (B, [L, P, P, I, I, P, I, I]),
}


def edge(x, y):
    return tuple(sorted((x, y)))


def replace(r, d, w1, w2, v1, v2, limit):
    """Compute a result using vertex sets and one entry per undirected edge."""
    rv, re = r
    dv, de = d
    if edge(w1, w2) not in re or v1 == v2 or v1 not in dv or (v2 not in dv):
        return None
    lev = max(max(de.values(), default=0), re[edge(w1, w2)]) + 1
    if de and lev >= limit:
        return None
    vertices = set(rv)
    edges = dict(re)
    del edges[edge(w1, w2)]
    mapping = {v1: w1, v2: w2}
    for v in sorted(dv - {v1, v2}):
        n = 0
        while n in vertices:
            n += 1
        vertices.add(n)
        mapping[v] = n
    for x, y in de:
        edges[edge(mapping[x], mapping[y])] = lev
    return (vertices, edges)


def test(library, limit):
    lib = C.CDLL(str(library))
    funcs = {}
    for name, (ret, args) in SIGNATURES.items():
        f = getattr(lib, 'rgc_' + name)
        f.restype = ret
        f.argtypes = args
        funcs[name] = f

    def f(name, *args):
        return funcs[name](*args)
    c = f('new_collection')
    assert c >= 0
    other = f('new_collection')
    assert other == c + 1
    f('delete_collection', other)
    extra = f('new_collection')
    assert extra == c + 2
    assert f('size', other) == -1
    assert f('max_level') == limit - 1
    completed_steps = 0
    assert f('first_graph', c) is None
    for bad in (None, b'', b'bad name', b'-', b'\xff', b'a.b', b'a/b'):
        assert not f('new_graph', c, bad, 2)
    assert not f('new_graph', c, b'negative', -1)
    assert not f('new_graph', -1, b'g', 1)
    assert f('size', c) == 0
    for badid, name in ((-1, b'g'), (c, None), (c, b'missing')):
        for fn in ('graph_new_node', 'first_node', 'graph_level', 'count_nodes', 'count_edges'):
            assert f(fn, badid, name) == -1
        for fn in ('first_edge', 'next_node'):
            assert f(fn, badid, name, 0) == -1
        for fn in ('next_edge', 'edge_level'):
            assert f(fn, badid, name, 0, 1) == -1
        assert not f('graph_delete_node', badid, name, 0)
        for fn in ('graph_add_edge', 'graph_delete_edge'):
            assert not f(fn, badid, name, 0, 1)
        f('delete_graph', badid, name)
    buf = C.create_string_buffer(b'Owned')
    assert f('new_graph', c, buf, 0)
    buf.value = b'Other'
    assert f('count_nodes', c, b'Owned') == 0
    f('delete_graph', c, b'Owned')
    rng = random.Random(20261030)
    model = {}
    counter = 0
    successful_replacements = 0
    failures = 0
    self_replacements = 0
    empty_donors = 0

    def verify(name):
        vertices, edges = model[name]
        assert f('count_nodes', c, name) == len(vertices)
        assert f('count_edges', c, name) == len(edges)
        assert f('graph_level', c, name) == max(edges.values(), default=0)
        got = []
        x = f('first_node', c, name)
        while x != -1:
            got.append(x)
            assert len(got) <= len(vertices)
            x = f('next_node', c, name, x)
        assert got == sorted(vertices)
        for q in (-2147483648, -1, 0, 1, 2, 3, 7, 13, 2147483647):
            assert f('next_node', c, name, q) == min((v for v in vertices if v > q), default=-1)
        for x in vertices | {-1, 2147483647}:
            ns = sorted((b if a == x else a for a, b in edges if a == x or b == x))
            got = []
            y = f('first_edge', c, name, x)
            while y != -1:
                got.append(y)
                assert len(got) <= len(ns)
                assert f('edge_level', c, name, x, y) == edges[edge(x, y)]
                y = f('next_edge', c, name, x, y)
            assert got == ns
            for q in (-1, 0, 1, 4, 9, 2147483647):
                assert f('next_edge', c, name, x, q) == min((v for v in ns if v > q), default=-1)
                assert f('edge_level', c, name, x, q) == edges.get(edge(x, q), -1)

    def add_graph(n):
        nonlocal counter
        name = f'G_{counter}'.encode()
        counter += 1
        assert f('new_graph', c, name, n)
        model[name] = (set(range(n)), {})
        for x in range(n):
            for y in range(x + 1, n):
                if rng.random() < 0.35:
                    assert f('graph_add_edge', c, name, x, y)
                    model[name][1][x, y] = 0
        return name
    for n in (0, 1, 2, 3, 5, 8):
        add_graph(n)
    for step in range(3500):
        if len(model) < 4:
            add_graph(rng.randrange(9))
        name = rng.choice(list(model))
        vertices, edges = model[name]
        action = rng.randrange(9)
        if action == 0:
            n = 0
            while n in vertices:
                n += 1
            assert f('graph_new_node', c, name) == n
            vertices.add(n)
        elif action == 1:
            x = rng.choice(sorted(vertices) + [-1, 99])
            assert f('graph_delete_node', c, name, x) == (x in vertices)
            vertices.discard(x)
            for e in list(edges):
                if x in e:
                    del edges[e]
        elif action in (2, 3):
            x = rng.choice(sorted(vertices) + [-1, 99])
            y = rng.choice(sorted(vertices) + [-1, 99])
            e = edge(x, y)
            if action == 2:
                ok = x in vertices and y in vertices and (x != y) and (e not in edges)
                assert f('graph_add_edge', c, name, x, y) == ok
                if ok:
                    edges[e] = 0
            else:
                ok = e in edges
                assert f('graph_delete_edge', c, name, x, y) == ok
                if ok:
                    del edges[e]
        elif action in (4, 5, 6):
            donor = name if rng.random() < 0.3 else rng.choice(list(model))
            dv, de = model[donor]
            if len(vertices) + len(dv) > 50:
                continue
            w1, w2 = rng.choice(list(edges)) if edges and rng.random() < 0.8 else (-1, 99)
            if rng.random() < 0.5:
                w1, w2 = (w2, w1)
            v1, v2 = rng.sample(sorted(dv), 2) if len(dv) >= 2 and rng.random() < 0.9 else (-1, -1)
            result = f'G_{counter}'.encode()
            counter += 1
            expect = replace(model[name], model[donor], w1, w2, v1, v2, limit)
            if rng.random() < 0.1:
                result = rng.choice((None, b'', b'bad name', name, donor))
                expect = None
            actual = f('replace_edge', c, result, name, w1, w2, donor, v1, v2)
            assert actual == (expect is not None), (limit, step, 'replace result')
            verify(name)
            verify(donor)
            if expect is not None:
                model[result] = expect
                verify(result)
                successful_replacements += 1
                self_replacements += donor == name
                empty_donors += not de
            else:
                failures += 1
        elif action == 7:
            assert not f('new_graph', c, name, 3)
        elif len(model) > 6:
            f('delete_graph', c, name)
            del model[name]
            assert f('count_nodes', c, name) == -1
        else:
            add_graph(rng.randrange(9))
        completed_steps += 1
        if name in model:
            verify(name)
        while len(model) > 18:
            discard = rng.choice(list(model))
            f('delete_graph', c, discard)
            del model[discard]
        assert f('size', c) == len(model)
        if step % 50 == 0:
            names = []
            s = f('first_graph', c)
            while s is not None:
                names.append(s)
                assert len(names) <= len(model)
                s = f('next_graph', c, s)
            assert names == sorted(model)
            for n in model:
                verify(n)
    for n in model:
        verify(n)
    f('delete_collection', c)
    assert f('size', c) == -1
    f('delete_collection', extra)
    print(
        f'PASS LEVEL_MAX={limit}: {completed_steps} model-based operations; '
        f'replacements {successful_replacements} accepted/{failures} rejected, '
        f'{self_replacements} self, {empty_donors} edgeless donors',
        flush=True,
    )
