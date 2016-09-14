

class Classifier
{
	public:
	Classifier(const vector<string> &_class_list) : class_list(_class_list) {}

	// Run training on a given dataset.
	virtual void train(const Dataset &filenames, string algo) = 0;

	// Classify a single image.
	virtual string classify(const string &filename) = 0;

	// Load in a trained model.
	virtual void load_model() = 0;

	// Loop through all test images, hiding correct labels and checking if we get them right.
	virtual void test(const Dataset &filenames, string algo) = 0;

	protected:
	vector<string> class_list;
};
