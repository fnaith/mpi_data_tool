from random import randint

def generate_discrete_values(nInstances, nFeatures, fileName):
  with open(fileName, "wb") as output:
    header = ["class"]
    for i in range(nFeatures):
      header.append(str(i+1))
    output.write(",v".join(header))
    output.write("\n")
    for _ in range(nInstances):
      line = [str(randint(0, 1))]
      for i in range(nFeatures):
        line.append(str(randint(-1, 1) * 2))
      output.write(",".join(line))
      output.write("\n")
# for i in [1,2,3,4,5,7]:
  # generate_discrete_values(10**i, 100, "g1_" + str(i))
# for i in [3,4,5]:
  # generate_discrete_values(100, 10**i, "g2_" + str(i))
