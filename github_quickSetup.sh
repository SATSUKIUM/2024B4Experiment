// ... or create a new repository on the command line

#echo "# sample" >> README.md
git init
git add README.md
git commit -m "first commit"
git branch -M main
git remote add origin https://github.com/SATSUKIUM/2024B4Experiment.git
pit push -u origin main

// ... or push an existing repository from the command line

#git remote add origin https://github.com/your-account/sample.git
#git branch -M main
#git push -u orign main