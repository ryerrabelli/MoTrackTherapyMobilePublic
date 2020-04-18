'''
import boto3
from boto.mturk.connection import MTurkConnection
from boto.mturk.question import HTMLQuestion
from boto.mturk.layoutparam import LayoutParameter
from boto.mturk.layoutparam import LayoutParameters
import json

# Create your connection to MTurk
mtc = MTurkConnection(aws_access_key_id='AKIAIBPHQKOJQZULHJSA',
aws_secret_access_key='2EDgdoD4lFrAUd4NHqWnF9qoQBYp1ekV6CVlhUTS',
host='mechanicalturk.sandbox.amazonaws.com') #host='mechanicalturk.amazonaws.com')
account_balance = mtc.get_account_balance()[0]
print("You have a balance of: {}".format(account_balance))
'''

import boto3
import json

region_name = 'us-east-1'
aws_access_key_id = 'AKIAIBPHQKOJQZULHJSA'
aws_secret_access_key = '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'

endpoint_url = 'https://mturk-requester-sandbox.us-east-1.amazonaws.com'

# Uncomment this line to use in production
# endpoint_url = 'https://mturk-requester.us-east-1.amazonaws.com'

mtc = boto3.client(
    'mturk',
    endpoint_url=endpoint_url,
    region_name=region_name,
    aws_access_key_id=aws_access_key_id,
    aws_secret_access_key=aws_secret_access_key,
)

# This will return $10,000.00 in the MTurk Developer Sandbox
print(mtc.get_account_balance()['AvailableBalance'])


# This is the value you received when you created the HIT
# You can also retrieve HIT IDs by calling GetReviewableHITs
# and SearchHITs. See the links to read more about these APIs.
hit_id = "386T3MLZLNVRU564VQVZSIKA8D580B"
result = mtc.get_assignments(hit_id)
assignment = result[0]
worker_id = assignment.WorkerId
for answer in assignment.answers[0]:
  if answer.qid == 'annotation_data':
    worker_answer = json.loads(answer.fields[0])

print("The Worker with ID {} gave the answer {}".format(worker_id, worker_answer))
left = worker_answer[0]['left']
top  = worker_answer[0]['top']
print("The top and left coordinates are {} and {}".format(top, left))