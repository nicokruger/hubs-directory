import * as cdk from 'aws-cdk-lib';

import {
  aws_s3 as s3,
  aws_s3_deployment as s3_deployment,
  aws_route53 as route53,
  aws_route53_targets as route53_targets,
  aws_cloudfront as cloudfront,
  aws_cloudfront_origins as origins,
  aws_certificatemanager as acm,
} from 'aws-cdk-lib';
import { Construct } from 'constructs';
// import * as sqs from 'aws-cdk-lib/aws-sqs';

export class CdkStack extends cdk.Stack {
  constructor(scope: Construct, id: string, props?: cdk.StackProps) {
    super(scope, id, props);

    // Deploy webapp to S3 bucket and host it on
    // CloudFront CDN and link Route53 domain
    // (hubs.inmytree.co.za) to it.
    const webapp = new s3.Bucket(this, 'webapp', {
      bucketName: 'hubs.inmytree.co.za',
      websiteIndexDocument: 'index.html',
      websiteErrorDocument: 'index.html',
      publicReadAccess: true,
      removalPolicy: cdk.RemovalPolicy.DESTROY,
    });

    new s3_deployment.BucketDeployment(this, 'webapp-deploy', {
      sources: [s3_deployment.Source.asset('../webapp/dist/')],
      destinationBucket: webapp,
    });

    // create cloudfront distribution
    if (!process.env.CLOUDFRONT_CERTIFICATE_ARN) {
      throw new Error('CLOUDFRONT_CERTIFICATE_ARN environment variable not set');
    }
    const certificateArn = process.env.CLOUDFRONT_CERTIFICATE_ARN;
    const distribution = new cloudfront.Distribution(this, 'webapp-distribution', {
      priceClass: cloudfront.PriceClass.PRICE_CLASS_ALL,
      defaultBehavior: { origin: new origins.S3Origin(webapp) },
      domainNames: ['hubs.inmytree.co.za'],
      certificate: acm.Certificate.fromCertificateArn(this, 'webapp-certificate', certificateArn),
    });

    const zone = route53.HostedZone.fromLookup(this, 'zone', {
      domainName: 'inmytree.co.za',
    });

    new route53.ARecord(this, 'webapp-record', {
      zone,
      recordName: 'hubs.inmytree.co.za',
      target: route53.RecordTarget.fromAlias(new route53_targets.CloudFrontTarget(distribution)),
    });



  }
}
