from graphqlclient import GraphQLClient
client = GraphQLClient('https://api.labelbox.com/graphql')
client.inject_token('Bearer <API_KEY_HERE>')

data = client.execute('''
  query {
    projects {
      name,
      labels(first:1) {
        id,
        label
      }
    }
  }
''')

{
  projects(first: 1) {
    name
    labels(first: 1) {
      id = cjwal2mivl2es0795yptwsl47
      label
    }
  }
}

{
  "data": {
    "createPredictionModel": {
      "id": "cjzftufqrmdxr0848gwlt9jfp"
    }
  }
}

{
  "data": {
    "updateProject": {
      "id": "cjwal2mivl2es0795yptwsl47"
    }
  }
}

print(data)