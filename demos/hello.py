from sorted_set import SortedSet

ss = SortedSet()
ss.add(1, 1.1)
ss.add(2, 2.2)

print(ss.get_score(1))
print(ss.get_score(2))

ss.rem(1)
ss.rem(2)

del ss

foo = SortedSet()
for i in range(100):
    foo.add(i, float(i))

for i in range(55, 66):
    print(foo.get_score(i))

for i in range(11, 22):
    print(foo.get_rank(i))
