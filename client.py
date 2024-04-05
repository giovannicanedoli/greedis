import redis
import time

r = redis.Redis(host="127.0.0.1", port=7379)

print("GETTING KEY")
print(r.get("forever"))

print("SETTING KEY")
print(r.set("forever", "value"))


print("SETTING KEY EX=10")
print(r.set("key", "value", ex=1000000))

print("1->12")
for i in range(12):
    print(r.get("key"))
    time.sleep(0.5)

print("non existing key")
print(r.get("non-existing-key"))
