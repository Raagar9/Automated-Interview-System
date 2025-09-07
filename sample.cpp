#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <Eigen/Dense>
#include <algorithm>
#include <cctype>
#include <cmath>

using namespace Eigen;

// Function to extract text from output file, excluding [BLANK_AUDIO]
std::string extractText(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string line, content;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (line.find("[BLANK_AUDIO]") == std::string::npos) {
                content += line + "\n";  // Add valid line to content
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open the file: " << filePath << std::endl;
    }

    return content;
}

// Function to convert a string to lowercase
std::string to_lowercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Function to tokenize a sentence
std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string token;
    for (char ch : text) {
        if (isspace(ch)) {
            if (!token.empty()) tokens.push_back(token);
            token.clear();
        }
        else {
            token += ch;
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

// Function to calculate cosine similarity between two vectors
double cosine_similarity(const VectorXd& vec1, const VectorXd& vec2) {
    if (vec1.size() == 0 || vec2.size() == 0) return 0.0; // Handle empty vectors
    double dot_product = vec1.dot(vec2);
    double norm_vec1 = vec1.norm(); // norm => magnitude
    double norm_vec2 = vec2.norm();
    return (norm_vec1 == 0 || norm_vec2 == 0) ? 0.0 : (dot_product / (norm_vec1 * norm_vec2));
}

// Function to calculate term frequency (TF)
double term_frequency(const std::string& term, const std::vector<std::string>& tokens) {
    int term_count = std::count(tokens.begin(), tokens.end(), term);
    return static_cast<double>(term_count) / tokens.size();
}

// Function to calculate inverse document frequency (IDF)
double inverse_document_frequency(const std::string& term, const std::vector<std::vector<std::string>>& all_documents) {
    int doc_count = 0;
    for (const auto& doc : all_documents) {
        if (std::find(doc.begin(), doc.end(), term) != doc.end()) {
            doc_count++;
        }
    }
    return log(static_cast<double>(all_documents.size()) / (1 + doc_count));  // adding 1 to avoid division by zero
}

// Function to create a TF-IDF vector for a sentence
VectorXd sentence_to_tfidf_vector(const std::vector<std::string>& tokens, const std::map<std::string, int>& vocab, const std::vector<std::vector<std::string>>& all_documents) {
    VectorXd vec = VectorXd::Zero(vocab.size());
    for (const std::string& token : tokens) {
        if (vocab.find(token) != vocab.end()) {
            double tf = term_frequency(token, tokens);
            double idf = inverse_document_frequency(token, all_documents);
            vec(vocab.at(token)) = tf * idf;
        }
    }
    return vec;
}

// Function to load questions and responses from a text file
std::map<std::string, std::string> load_responses(const std::string& filename, std::vector<std::string>& questions) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the text file.\n";
        exit(EXIT_FAILURE);
    }

    std::map<std::string, std::string> question_response_map;
    std::string question, response;

    // Load question-response pairs
    while (std::getline(file, question) && std::getline(file, response)) {
        if (!question.empty() && !response.empty()) {
            question_response_map[question] = response;
            questions.push_back(question);  // Store the question for later use
        }
    }

    file.close();
    return question_response_map;
}

// Function to find the best response based on cosine similarity
std::pair<std::string, double> find_best_response(const std::string& input, const std::map<std::string, std::string>& question_response_map) {
    std::vector<std::string> input_tokens = tokenize(input);

    // Build vocabulary and document set (responses)
    std::map<std::string, int> vocab;
    std::vector<std::vector<std::string>> all_documents;
    int index = 0;

    for (const auto& entry : question_response_map) {
        std::vector<std::string> response_tokens = tokenize(entry.second);  // Use responses for vocab
        all_documents.push_back(response_tokens);

        for (const std::string& token : response_tokens) {
            if (vocab.find(token) == vocab.end()) {
                vocab[token] = index++;
            }
        }
    }

    // Convert input to TF-IDF vector
    VectorXd input_vector = sentence_to_tfidf_vector(input_tokens, vocab, all_documents);

    // Compare with all responses
    double max_similarity = -1;
    std::string best_response;
    for (const auto& entry : question_response_map) {
        std::vector<std::string> response_tokens = tokenize(entry.second);  // Tokenize the response
        VectorXd response_vector = sentence_to_tfidf_vector(response_tokens, vocab, all_documents);
        double similarity = cosine_similarity(input_vector, response_vector);

        if (similarity > max_similarity) {
            max_similarity = similarity;
            best_response = entry.second;  // Update the best response with the matching response
        }
    }

    return { best_response, max_similarity };  // Return the best response and its similarity score
}

// Function to evaluate a response based on keywords and ideal answer
double evaluate_response(const std::string& response, const std::string& ideal_answer, const std::vector<std::string>& keywords) {
    int keyword_matches = 0;
    std::vector<std::string> response_tokens = tokenize(response);
    std::vector<std::string> ideal_tokens = tokenize(ideal_answer);

    // Check for keyword matches in the response
    for (const std::string& keyword : keywords) {
        if (std::find(response_tokens.begin(), response_tokens.end(), keyword) != response_tokens.end()) {
            keyword_matches++;
        }
    }

    // Calculate match percentage based on the number of keyword matches
    double match_percentage = static_cast<double>(keyword_matches) / keywords.size();

    // You can customize this rating formula to suit your evaluation needs
    double length_factor = std::min(static_cast<double>(response_tokens.size()) / ideal_tokens.size(), 1.0);
    return match_percentage * 0.7 + length_factor * 0.3;  // Weight match percentage higher than length match
}

int main() {
    // Paths to the batch files and output text file
    const char* recordBatchFilePath = "C:\\CollegeStuff\\TY\\AI\\CP\\run_record_audio.bat";
    const char* whisperBatchFilePath = "C:\\CollegeStuff\\TY\\AI\\CP\\run_whisper.bat";
    const std::string outputFilePath = "C:\\CollegeStuff\\TY\\AI\\CP\\output.wav.txt";

    // Load the question-response map and questions
    std::vector<std::string> questions;
    std::map<std::string, std::string> question_response_map = load_responses("nlp.txt", questions);

    // Display the first question from the loaded questions
    if (!questions.empty()) {
        std::cout << "Question: " << questions[0] << std::endl;  // Display the first question
    } else {
        std::cerr << "No questions found in the file." << std::endl;
        return EXIT_FAILURE;
    }

    // Execute the recording batch file
    std::cout << "Executing recording batch file..." << std::endl;
    int recordResult = system(recordBatchFilePath);

    // Check the result of the recording execution
    if (recordResult == 0) {
        std::cout << "Recording batch file executed successfully." << std::endl;

        // Execute the whisper batch file
        std::cout << "Executing whisper batch file..." << std::endl;
        int whisperResult = system(whisperBatchFilePath);

        // Check the result of the whisper execution
        if (whisperResult == 0) {
            std::cout << "Whisper batch file executed successfully." << std::endl;

            // Extract and process the text from the output file
            std::string extracted_text = extractText(outputFilePath);
            std::cout << "Extracted Text:\n" << extracted_text << std::endl;

            // // Find and display the best response based on extracted text
            // auto [best_response, similarity_score] = find_best_response_with_tfidf(extracted_text, question_response_map);
            // std::cout << "Best Response: " << best_response << std::endl;
            // std::cout << "Cosine Similarity Score: " << similarity_score << std::endl;  // Display the score

            // Evaluate the extracted response against an ideal answer
            // const std::string ideal_answer = "It is important to always value knowledge and stay committed to learning.";
            // double final_rating = evaluate_response(extracted_text, ideal_answer, keywords);
            // std::cout << "Response Evaluation Rating: " << final_rating << std::endl;  // Display the final rating

            auto [best_response, similarity_score] = find_best_response(extracted_text, question_response_map);
            std::cout << "Best Response (TF-IDF based): " << best_response << std::endl;
            std::cout << "Cosine Similarity Score (TF-IDF): " << similarity_score << std::endl;  // Display the score

            // Define keywords for evaluation
            std::vector<std::string> keywords = { "artificial", "intelligence", "subject" };

            // Evaluate the extracted response against an ideal answer
            const std::string ideal_answer = "Artificial Intelligence is my favourite subject.";
            double final_rating = evaluate_response(extracted_text, ideal_answer, keywords);
            std::cout << "Response Evaluation Rating: " << final_rating << std::endl;  // Display the final rating
        } else {
            std::cerr << "Failed to execute whisper batch file. Error code: " << whisperResult << std::endl;
        }
    } else {
        std::cerr << "Failed to execute recording batch file. Error code: " << recordResult << std::endl;
    }

    return 0;
}